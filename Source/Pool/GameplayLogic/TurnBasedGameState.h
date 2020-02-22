// Copyright 2019 Andrei Vikarchuk.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "GameplayLogic/TurnBasedGameHandler.h"
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
    virtual bool IsMyTurn(const ITurnBasedPlayer* Player) override;
    virtual void EndCurrentTurn() override {};

protected:
    TArray<ITurnBasedPlayer*> TurnBasedPlayers;

    virtual void AddPlayerState(APlayerState* PlayerState) override;
    virtual void RemovePlayerState(APlayerState* PlayerState) override;

    UPROPERTY(Replicated)
    uint8 PlayerIndexTurn;
};
