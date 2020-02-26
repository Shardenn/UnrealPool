// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "GameplayLogic/Interfaces/TurnBasedPlayer.h"
#include "AmericanPool/EightPoolBallType.h"
#include "PoolPlayerState.generated.h"

/**
 *
 */
UCLASS()
class POOL_API APoolPlayerState : public APlayerState, public ITurnBasedPlayer
{
    GENERATED_BODY()

public:
    UFUNCTION(Server, Reliable, WithValidation)
    void Server_ToggleReady();

    //UFUNCTION(Server, Reliable, WithValidation)
    virtual void SetIsMyTurn(const bool bInMyTurn) noexcept override;

    UFUNCTION(BlueprintPure)
    virtual bool GetIsMyTurn() const noexcept override { return bMyTurn; }

    UFUNCTION(BlueprintPure)
    bool GetIsReady() const { return bIsReady; }

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_HandleFrameWon();

    UFUNCTION(BlueprintPure)
    uint8 GetFramesWon() { return FramesWon; }

    virtual void OnTurnStarted() override {};
    virtual void OnTurnEnded() override {};
protected:
    UPROPERTY(replicated)
    bool bIsReady = false;

    UPROPERTY(replicated)
    bool bMyTurn = false;

    UPROPERTY(replicated)
    uint8 FramesWon = 0;
};
