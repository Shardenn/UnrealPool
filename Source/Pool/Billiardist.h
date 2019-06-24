// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/Engine.h"
#include "Table.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SplineComponent.h"
#include "Billiardist.generated.h"

/* for fast copy-paste in future
switch (m_eState)
    {
        case FBilliardistState::WALKING:
        {
            break;
        }
        case FBilliardistState::PICKING:
        {
            break;
        }
        case FBilliardistState::AIMING:
        {
            break;
        }
        case FBilliardistState::OBSERVING:
        {
            break;
        }
        case FBilliardistState::EXAMINING:
        {
            break;
        }
    }
*/



UENUM(BlueprintType)
enum class FBilliardistState : uint8
{
    WALKING     UMETA(DisplayName = "Walking"),     // just walking around the table, examining
    PICKING     UMETA(DisplayName = "Picking"),     // if we are playing RU billiard, we can pick any ball for the shot
    AIMING      UMETA(DisplayName = "Aiming"),      // when a ball is picked, we aim for the shot, holding the cue near the ball
    OBSERVING   UMETA(DisplayName = "Observing"),  // observing the balls after a shot
    EXAMINING   UMETA(DisplayName = "Examinging"),  // watching from the top of the table
    POSSIBLE_STATES_NUMBER = 5 UMETA(DisplayName = "Possible values number")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerStateChange, FBilliardistState, NewState);

UCLASS()
class POOL_API ABilliardist : public ACharacter
{
    GENERATED_BODY()

public:
    // Sets default values for this character's properties
    ABilliardist();

    UPROPERTY(BlueprintAssignable)
    FOnPlayerStateChange OnStateChange;

    UFUNCTION(BlueprintCallable, Category = "Billiardist Character", meta = (DisplayName = "Set Table"))
    void SetTable(ATable* NewTable);
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintPure, Category = "Billiardist Character", meta = (DisplayName = "Get Move Speed"))
    float GetMoveSpeed() { return m_fMoveSpeed; }
    UFUNCTION(BlueprintPure, Category = "Billiardist Character", meta = (DisplayName = "Get Spline"))
    USplineComponent* GetSpline() { return m_pSplinePath; }
    
    FVector m_fCurrentMoveDirection = FVector(0);

    UFUNCTION(BlueprintCallable, Category = "Billiardist Character", meta = (DisplayName = "Handle action input"))
    void ActionPressHandle();

    UFUNCTION(BlueprintCallable, Category = "Billiardist Character", meta = (DisplayName = "Handle return input"))
    void ReturnPressHandle();

    UFUNCTION(BlueprintCallable, Category = "Billiardist Character", meta = (DisplayName = "Handle examining input"))
    void ExaminingPressHandle();

    UFUNCTION(BlueprintCallable, Category = "Billiardist Character", meta = (DisplayName = "Set State"))
    void SetState(FBilliardistState NewState);

    UFUNCTION(BlueprintPure, Category = "Billiardist Character", meta = (DisplayName = "Get State"))
    FBilliardistState GetState() { return m_eState; }

    UPROPERTY(EditAnywhere, Category = "Gameplay", meta = (DisplayName = "Hit strength"))
    float m_fHitStrength = 0.0f;

    UPROPERTY(EditAnywhere, Category = "Gameplay", meta = (DisplayName = "Hit strength multiplier"))
    float m_fHitStrengthMultiplier = 10.0f;
protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    // does not needs to be replicated for movement (only m_pSpline does)
    // but may be needed for replication later
    UPROPERTY(/*Replicated, */EditAnywhere, Category = "Billiardist Character", meta = (DisplayName = "Assigned billiard table"))
    ATable* m_pTable = nullptr;
    UPROPERTY(EditAnywhere, Category = "Billiardist Character", meta = (DisplayName = "Move speed"))
    float m_fMoveSpeed = 1.0f;

    
private:
    UPROPERTY(Replicated) // needed for replication, tested
    USplineComponent* m_pSplinePath { nullptr };
    
    UPROPERTY(Replicated)
    FBilliardistState m_eState { FBilliardistState::WALKING };
    UPROPERTY(Replicated)
    FBilliardistState m_ePreviousState { FBilliardistState::WALKING };

    UFUNCTION(server, reliable, WithValidation)
    void Server_SetTable(ATable* NewTable);
    UFUNCTION(server, reliable, WithValidation)
    void Server_SetState(FBilliardistState NewState);
};
