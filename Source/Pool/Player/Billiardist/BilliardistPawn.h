// Copyright 2019 Andrei Vikarchuk.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"

#include "BilliardistStates.h"

#include "BilliardistPawn.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerStateChange, FBilliardistState, NewState);

UCLASS()
class POOL_API ABilliardistPawn : public APawn
{
    GENERATED_BODY()

public:
    ABilliardistPawn();

    virtual void Tick(float DeltaTime) override;

    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    UFUNCTION(BlueprintCallable)
    void SetSpline(class USplineComponent* Spline);
    
    UPROPERTY(BlueprintAssignable)
    FOnPlayerStateChange OnStateChange;

    UFUNCTION(BlueprintCallable)
    void SetState(const FBilliardistState& NewState);

    UFUNCTION(BlueprintPure)
    FBilliardistState GetState() { return State; }

    bool TryInitializePlayerState();

    UFUNCTION(BlueprintPure)
    float GetMaxHitStrength();
    UFUNCTION(BlueprintPure)
    float GetCurrentHitStrength();

    virtual void NotifyTurnUpdate(bool NewTurn);
    UFUNCTION(Client, Reliable)
    virtual void Client_NotifyBallInHand(bool IsInHand);
protected:
    virtual void BeginPlay() override;

    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
    FBilliardistState State = FBilliardistState::WALKING;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    class APoolPlayerState* BillPlayerState = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    class ABall* SelectedBall = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<class ABall> GhostBallClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    class ABall* GhostHandedBall = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    class UBilliardistMovementComponent* MovementComponent = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    class UBilliardistAimingComponent* AimingComponent = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bAdjustingHitStrength = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bAdjustingZoom = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    class UBilliardistReplicationComponent* ReplicationComponent = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    class USpringArmComponent* SpringArm = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    class UCameraComponent* Camera = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    class UStaticMeshComponent* Mesh = nullptr;
    
    // override this for the event when a player picks a ball
    virtual void HandleBallSelected(class ABall* Ball);
    virtual void HandleFinishedAiming();

    void LaunchBall(ABall* Ball, const FVector& Velocity);
private:
    // Checks current GhostHandedBall for overlaps with static mesh components.
    // If there is no GhostHandedBall - returns false!!!
    bool IsCueBallPlacementValid() const;
    void TryPlaceCueBall(class APoolPlayerState* InPlayerState);

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_SetState(const FBilliardistState& NewState);

#pragma region InputBindedFunctions
    UFUNCTION()
    void MoveForward(float Value);
    UFUNCTION()
    void MoveRight(float Value);
    UFUNCTION()
    void Turn(float Value);
    UFUNCTION()
    void LookUp(float Value);

    UFUNCTION()
    void StartedZoomAdjustement();
    UFUNCTION()
    void FinishedZoomAdjustement();

    UFUNCTION()
    void ActionPressHandle();
    UFUNCTION()
    void ActionReleaseHandle();
    UFUNCTION()
    void ReturnPressHandle();
    //UFUNCTION()
    //void ExaminingPressHandle();
    UFUNCTION()
    void ReadyStateToggle();
#pragma endregion
};
