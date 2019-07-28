// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "PoolPlayerState.generated.h"

/**
 *
 */
UCLASS()
class POOL_API APoolPlayerState : public APlayerState
{
    GENERATED_BODY()

public:
    UFUNCTION(Server, Reliable, WithValidation)
    void ToggleReady();

    //UFUNCTION(Server, Reliable, WithValidation)
    void SetIsMyTurn(bool bInMyTurn);

    void SetIsBallInHand(bool InbBallInHand);

    UFUNCTION(BlueprintPure)
    bool GetIsMyTurn() const { return bMyTurn; }

    UFUNCTION(BlueprintPure)
    bool GetIsReady() const { return bIsReady; }

    UFUNCTION(BlueprintPure)
    bool GetIsBallInHand() const { return bBallInHand; }

protected:
    UPROPERTY(replicated)
    bool bIsReady = false;

    UPROPERTY(replicated)
    bool bMyTurn = false;

    UPROPERTY(replicated)
    bool bBallInHand = false;
};
