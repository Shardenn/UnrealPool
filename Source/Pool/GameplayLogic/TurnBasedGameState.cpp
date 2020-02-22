// Copyright 2019 Andrei Vikarchuk.


#include "TurnBasedGameState.h"
#include "GameFramework/PlayerState.h"
#include "GameplayLogic/TurnBasedPlayer.h"

bool ATurnBasedGameState::IsMyTurn(const ITurnBasedPlayer* Player)
{
    const auto CurrentPlayerTurn = TurnBasedPlayers[PlayerIndexTurn];
    return CurrentPlayerTurn == Player;
}

void ATurnBasedGameState::AddPlayerState(APlayerState* PlayerState)
{
    Super::AddPlayerState(PlayerState);
    
    ITurnBasedPlayer* TurnBasedPlayerState = Cast<ITurnBasedPlayer>(PlayerState);
    if (TurnBasedPlayerState)
    {
        TurnBasedPlayers.AddUnique(TurnBasedPlayerState);
    }
}

void ATurnBasedGameState::RemovePlayerState(APlayerState* PlayerState)
{
    Super::RemovePlayerState(PlayerState);

    ITurnBasedPlayer* TurnBasedPlayerState = Cast<ITurnBasedPlayer>(PlayerState);
    if (TurnBasedPlayerState)
    {
        for (int32 i = 0; i < TurnBasedPlayers.Num(); i++)
        {
            if (TurnBasedPlayers[i] == TurnBasedPlayerState)
            {
                TurnBasedPlayers.RemoveAt(i, 1);
                return;
            }
        }
    }
}

void ATurnBasedGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}