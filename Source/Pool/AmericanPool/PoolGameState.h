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

    UFUNCTION()
    void AddMovingBall(class UPrimitiveComponent* Comp, FName BoneName);
    UFUNCTION()
    void RemoveMovingBall(class UPrimitiveComponent* Comp, FName BoneName);

    bool RequestIsPlayerTurn(APlayerState* PlayerState);

protected:
    virtual void BeginPlay() override;

private:
    TArray<class ABall*> MovingBalls;

    UPROPERTY(Replicated)
    uint32 PlayerIndexTurn;
};
