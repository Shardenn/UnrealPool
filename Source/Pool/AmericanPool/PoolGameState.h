// Fill out your copyright notice in the Description page of Project Settings.

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
    void StartWatchingBallsMovement() { bWatchBallsMovement = true; }

    // they are attached to awake and sleep events in ABall class.
    // NOTE And they are attached only on the server.
    // So the function is only called on server.
    // Only works if balls are correctly processed on server
    // (like LaunchBall is called as server function)
    UFUNCTION()
    void AddMovingBall(class UPrimitiveComponent* Comp, FName BoneName);
    UFUNCTION()
    void RemoveMovingBall(class UPrimitiveComponent* Comp, FName BoneName);

    UFUNCTION(Server, Reliable, WithValidation)
    void SwitchTurn();

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

private:
    bool bWatchBallsMovement = false;
    TArray<class ABall*> MovingBalls;
    
    APoolPlayerState* PlayerWithCueBall = nullptr;

    void OnRep_UpdatePlayerStateTurn();
};
