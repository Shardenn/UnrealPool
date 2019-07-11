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

    bool IsMyTurn();

protected:
    UPROPERTY(replicated)
    bool bIsReady = false;

};
