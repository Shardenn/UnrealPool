// Copyright 2019 Andrei Vikarchuk.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "PoolGameState.generated.h"

/**
 *
 */
UCLASS()
class POOL_API APoolGameState : public AGameState
{
    GENERATED_BODY()

public:
    UFUNCTION(Server, Reliable, WithValidation)
    void SetPlayersReadyNum(uint32 PlayersReady);

    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
    int32 PlayersReadyNum = 0;

    // not Server. It is only called from LaunchBall, that is 
    // already on Server. Maybe make it server later
    UFUNCTION(Server, Reliable, WithValidation)
    void Server_StartWatchingBallsMovement();

    // they are attached to awake and sleep events in ABall class.
    // NOTE And they are attached only on the server.
    // So the function is only called on server.
    // Only works if balls are correctly processed on server
    // (like LaunchBall is called as server function)
    UFUNCTION()
    void AddMovingBall(class UPrimitiveComponent* Comp, FName BoneName);
    UFUNCTION()
    void RemoveMovingBall(class UPrimitiveComponent* Comp, FName BoneName);

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

    UFUNCTION()
    void OnFrameRestarted();

    UFUNCTION(Server, Reliable, WithValidation)
    void SwitchTurn();

    UFUNCTION(Server, Reliable, WithValidation)
    void HandleTurnEnd();

    UFUNCTION(Server, Reliable, WithValidation)
    void AssignFoul();

    // Removes the ball from the active ones
    UFUNCTION(Server, Reliable, WithValidation)
    void RegisterBall(class ABallAmerican* Ball);

    UFUNCTION(Server, Reliable, WithValidation)
    void GiveBallInHand(APoolPlayerState* PlayerState = nullptr);

    UFUNCTION(Server, Reliable, WithValidation)
    void TakeBallFromHand();

    bool RequestIsPlayerTurn(APlayerState* PlayerState);

    // TODO maybe not public?
    TArray<class ABall*> ActiveBalls;
    UPROPERTY(Replicated/*Using=OnRep_UpdatePlayerStateTurn*/)
    uint32 PlayerIndexTurn;
protected:
    virtual void BeginPlay() override;

    // returns true if the conditions for the win
    // are satisfied. (Like every ball is pocketed
    // BEFORE pocketed 8 ball).
    virtual bool DecideWinCondition();

    class ABallAmerican* CueBall = nullptr;

private:
    bool bWatchBallsMovement = false;
    bool bTableOpened = true;
    bool bBallsRackBroken = false;
    bool bPlayerFouled = false;
    bool bShouldSwitchTurn = true;

    // handle classes and their hiererchy
    TArray<class ABall*> MovingBalls;
    TArray<class ABallAmerican*> PocketedBalls;
    TArray<class ABallAmerican*> BallsHittedByTheCue;
    TArray<class ABallAmerican*> DroppedBalls;
    TArray<class ABallAmerican*> BallsPlayedOutOfGame;

    APoolPlayerState* PlayerWithCueBall = nullptr;

    void OnRep_UpdatePlayerStateTurn();

    void ClearTurnStateVariables();

    void HandleBlackBallOutOfPlay();
};
