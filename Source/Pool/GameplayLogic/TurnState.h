// Copyright 2019 Andrei Vikarchuk.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TurnState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBallStartsMoving, class ABall*, Ball);

/**
 *
 */
UCLASS()
class POOL_API UTurnState : public UObject
{
    GENERATED_BODY()

public:
    UTurnState();
    virtual ~UTurnState();

    UFUNCTION(BlueprintCallable)
    void ClearTurnStateVariables();

    UFUNCTION(BlueprintPure)
    const TArray<class ABall*>& GetMovingBalls() const { return m_MovingBalls; }
    UFUNCTION(BlueprintPure)
    const TArray<class ABall*>& GetPocketedBalls() const { return m_BallsPocketed; }
    UFUNCTION(BlueprintPure)
    const TArray<class ABall*>& GetDroppedBalls() const { return m_BallsDropped; }
    UFUNCTION(BlueprintPure)
    const TArray<class ABall*>& GetBallsHittedByTheCue() const { return m_BallsHittedByTheCue; }

protected:
    virtual bool IsSupportedForNetworking() const override { return true; }

    UPROPERTY(BlueprintAssignable)
    FOnBallStartsMoving OnBallStartsMoving;

    virtual void HandleBallPocketed();
    
    virtual void Server_HandleTurnEnd() = 0;
    virtual void AssignFoul() = 0;

    TArray<class ABall*> m_MovingBalls;
    TArray<class ABall*> m_BallsHittedByTheCue;
    TArray<class ABall*> m_BallsPocketed;
    TArray<class ABall*> m_BallsDropped;

    // they are attached to awake and sleep events in ABall class.
    // NOTE And they are attached only on the server.
    // So the function is only called on server.
    // Only works if balls are correctly processed on server
    // (like LaunchBall is called as server function)
    UFUNCTION()
    void AddMovingBall(const class UPrimitiveComponent* Comp, FName BoneName);
    UFUNCTION()
    void RemoveMovingBall(const class UPrimitiveComponent* Comp, FName BoneName);

    UFUNCTION()
    void OnBallOverlap(UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);

    UFUNCTION()
    void OnBallEndOverlap(UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex);

    UFUNCTION()
    void OnCueBallHit(UPrimitiveComponent* HitComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        FVector NormalImpulse,
        const FHitResult& Hit);

    bool m_bWatchBallsMovement = false;
    bool m_bPlayerFouled = false;

    void HandleBlackBallOutOfPlay();

    bool FindAndInitializeCueBall();

    //UFUNCTION(Server, Reliable, WithValidation)
    void Server_AssignFoul();

    void StartWatchingBallsMovement();
};
