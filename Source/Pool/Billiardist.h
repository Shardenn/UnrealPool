// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/Engine.h"
#include "Table.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SplineComponent.h"
#include "BilliardistController.h"
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


UCLASS()
class POOL_API ABilliardist : public ACharacter
{
    GENERATED_BODY()

public:
    // Sets default values for this character's properties
    ABilliardist();

    UFUNCTION(BlueprintCallable, Category = "Billiardist Character", meta = (DisplayName = "Set Table"))
    void SetTable(ATable* NewTable);
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintPure, Category = "Billiardist Character", meta = (DisplayName = "Get Move Speed"))
    float GetMoveSpeed() { return m_fMoveSpeed; }
    UFUNCTION(BlueprintPure, Category = "Billiardist Character", meta = (DisplayName = "Get Spline"))
    USplineComponent* GetSpline() { return m_pSplinePath; }
    
    FVector m_fCurrentMoveDirection = FVector(0);
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

    UFUNCTION(server, reliable, WithValidation)
    void Server_SetTable(ATable* NewTable);

    UFUNCTION(server, reliable, WithValidation)
    void Server_SubscribeToStateChange();

    UFUNCTION()
    void OnPlayerStateChanged(FBilliardistState newState);
};
