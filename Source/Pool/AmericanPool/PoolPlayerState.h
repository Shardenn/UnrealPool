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

    UFUNCTION(BlueprintCallable)
    bool GetIsMyTurn() const { return bMyTurn; }

    UFUNCTION(BlueprintCallable)
    bool GetIsReady() const { return bIsReady; }

protected:
    UPROPERTY(replicated)
    bool bIsReady = false;

    UPROPERTY(replicated)
    bool bMyTurn = false;
};
