// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/Engine.h"
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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerStateChange, FBilliardistState, NewState, FBilliardistState, OldState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSelectedBallUpdated, ABall*, NewBall);

UCLASS()
class POOL_API ABilliardist : public ACharacter
{
    GENERATED_BODY()

public:
    // Sets default values for this character's properties
    ABilliardist();
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

    UPROPERTY(BlueprintAssignable)
    FOnPlayerStateChange OnStateChange;
    UPROPERTY(BlueprintAssignable)
    FOnSelectedBallUpdated OnSelectedBallUpdate;

    UFUNCTION(BlueprintCallable, Category = "Billiardist | Setup")
    void Initialize(USplineComponent* Spline);

    UFUNCTION(BlueprintCallable, Category = "Billiardist")
    void SetState(FBilliardistState NewState);
    UFUNCTION(BlueprintPure, Category = "Billiardist")
    FBilliardistState GetState() { return BilliardistState; }

    UFUNCTION(BlueprintCallable, Category = "Billiardist")
    void SetSelectedBall(ABall* NewBall);
    UFUNCTION(BlueprintPure, Category = "Billiardist")
    ABall* GetSelectedBall() { return SelectedBall; }

    UFUNCTION(BlueprintPure, Category = "Billiardist")
    float GetMoveSpeed() { return MoveSpeed; }
    UFUNCTION(BlueprintPure, Category = "Billiardist")
    float GetAimStrengthAlpha() { return HitStrengthAlpha; }

    UFUNCTION(BlueprintCallable, Category = "Billiardist | Setup")
    void SetSplinePath(USplineComponent* NewSpline);
    UFUNCTION(BlueprintPure, Category = "Billiardist")
    USplineComponent* GetSpline() { return SplinePath; }
    
    UFUNCTION(BlueprintCallable, Category = "Billiardist")
    void LaunchBall(ABall* Ball, FVector Velocity);

    FVector CurrentMoveDirection = FVector(0);
protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    UFUNCTION(BlueprintImplementableEvent, Category = "Billiardist")
    void OnPlayerStateChanged(FBilliardistState NewState, FBilliardistState OldState);

    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite, Category = "Billiardist | Gameplay process") // needed for replication, tested
    USplineComponent* SplinePath { nullptr };
    
    UPROPERTY(ReplicatedUsing=OnRep_SelectedBallReplicated, VisibleAnywhere, BlueprintReadWrite, Category = "Billiardist | Gameplay process")
    ABall* SelectedBall { nullptr };
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Billiardist | Controls")
    float MoveSpeed = 200.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Billiardist | Controls")
    float MouseSenseX = 1.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Billiardist | Controls")
    float MouseSenseY = 1.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Billiardist | Hit strength")
    float HitStrengthMin = 50.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Billiardist | Hit strength")
    float HitStrengthMax = 500.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Billiardist | Hit strength")
    float HitStrengthChangeSpeed = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Billiardist | Hit strength")
    float HitStrengthChangeHigh = 2.0f;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Billiardist | Hit strength")
    float HitStrengthAlpha = 0.f; // from 0 to 1
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Billiardist | Hit strength")
    float CurrentHitStrength = HitStrengthMin;

    UPROPERTY(Replicated)
    FBilliardistState PreviousState { FBilliardistState::WALKING };
    UPROPERTY(ReplicatedUsing=OnRep_StateReplicated)
    FBilliardistState BilliardistState { FBilliardistState::WALKING };
private:
    float DistanceAlongSpline{ 0.0f };
    // for calculating aiming strength
    bool StrengthIncreasing = true;

    UFUNCTION(reliable, server, WithValidation)
    void Server_SetState(FBilliardistState NewState);

    UFUNCTION()
    void OnRep_StateReplicated();

    UFUNCTION(reliable, server, WithValidation)
    void Server_SetSelectedBall(ABall* NewBall);

    UFUNCTION()
    void OnRep_SelectedBallReplicated();

    void MovePlayer(FVector NewLocation);
    UFUNCTION(reliable, server, WithValidation)
    void Server_MovePlayer(FVector NewLocation);

    // launches ball on server with multicasting
    UFUNCTION(reliable, server, WithValidation)
    void Server_LaunchBall(ABall* Ball, FVector Velocity);

    UFUNCTION()
    void MoveForward(float Value);
    UFUNCTION()
    void MoveRight(float Value);
    UFUNCTION()
    void Turn(float Value);
    UFUNCTION()
    void LookUp(float Value);
    UFUNCTION()
    void ActionPressHandle();
    UFUNCTION()
    void ReturnPressHandle();
    UFUNCTION()
    void ExaminingPressHandle();
};
