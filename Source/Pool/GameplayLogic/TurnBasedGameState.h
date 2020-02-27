// Copyright 2019 Andrei Vikarchuk.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "GameplayLogic/Interfaces/TurnBasedGameHandler.h"
#include "TurnBasedGameState.generated.h"

class ITurnBasedPlayer;

/**
 *
 */
UCLASS()
class POOL_API ATurnBasedGameState : public AGameState, public ITurnBasedGameHandler
{
    GENERATED_BODY()

public:
    virtual bool IsMyTurn(const TScriptInterface<ITurnBasedPlayer>& Player) override;

    virtual void SwitchTurn() override;

    UPROPERTY(BlueprintAssignable)
    FOnTurnEnd OnTurnEnd;
protected:
    TArray<TScriptInterface<ITurnBasedPlayer>> TurnBasedPlayers;

    virtual void AddPlayerState(APlayerState* PlayerState) override;
    virtual void RemovePlayerState(APlayerState* PlayerState) override;

    UPROPERTY(Replicated)
    uint8 PlayerIndexTurn;

    // Override THIS in order to define turn end behavior
    virtual void SwitchTurn_Internal() override;

    void BroadcastOnTurnEnd();
    // Override THIS in order to define turn end broadcast behavior
    // (probably you don't need to)
    virtual void BroadcastOnTurnEnd_Internal();

private:
    UFUNCTION(Server, Reliable, WithValidation)
    void Server_SwitchTurn();

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_BroadcastOnTurnEnd();

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_BroadcastOnTurnEnd();
};
