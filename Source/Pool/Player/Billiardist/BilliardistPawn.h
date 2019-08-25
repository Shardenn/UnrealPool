// Copyright 2019 Andrei Vikarchuk.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "BilliardistPawn.generated.h"

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
protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    class UBilliardistMovementComponent* MovementComponent = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    class UBilliardistAimingComponent* AimingComponent = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    class UBilliardistReplicationComponent* ReplicationComponent = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    class USpringArmComponent* SpringArm = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    class UCameraComponent* Camera = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    class UStaticMeshComponent* Mesh = nullptr;
    //class APoolPlayerState* PlayerState = nullptr;
private:
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
    //UFUNCTION()
    //void ExaminingPressHandle();
    UFUNCTION()
    void ReadyStateToggle();
};
