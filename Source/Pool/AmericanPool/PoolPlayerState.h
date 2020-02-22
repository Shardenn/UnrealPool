// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "GameplayLogic/TurnBasedPlayer.h"
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

    UFUNCTION(Server, Reliable, WithValidation)
    void PlaceCueBall(const FVector& TablePoint) const;

    //UFUNCTION(Server, Reliable, WithValidation)
    virtual void SetIsMyTurn(const bool bInMyTurn) noexcept override;

    // if cue ball is valid - ball in hand
    // if nullptr is given -> ball is not in hand
    //UFUNCTION(Server, Reliable, WithValidation)
    void SetBallInHand(class ABall* CueBall);

    void AssignBallType(FBallType Type);

    UFUNCTION(BlueprintPure)
    virtual bool GetIsMyTurn() const noexcept override { return bMyTurn; }

    UFUNCTION(BlueprintPure)
    bool GetIsReady() const { return bIsReady; }

    UFUNCTION(BlueprintPure)
    bool GetIsBallInHand() const { return CueBallHanded != nullptr; }

    UFUNCTION(BlueprintPure)
    FBallType GetAssignedBallType() { return AssignedBallType; }

    UFUNCTION(Server, Reliable, WithValidation)
    void HandleFrameWon();

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
    bool bBallInHand = false;

    // when ball in hand, it will contain a pointer to the cue ball
    UPROPERTY(replicated)
    class ABall* CueBallHanded = nullptr;

    UPROPERTY(replicated)
    FBallType AssignedBallType = FBallType::NotInitialized;

    UPROPERTY(replicated)
    uint8 FramesWon = 0;
};
