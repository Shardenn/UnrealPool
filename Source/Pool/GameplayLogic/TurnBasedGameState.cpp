// Copyright 2019 Andrei Vikarchuk.


#include "TurnBasedGameState.h"

#include "GameFramework/PlayerState.h"
#include "GameplayLogic/Interfaces/TurnBasedPlayer.h"

#include "Pool.h"

bool ATurnBasedGameState::IsMyTurn(const TScriptInterface<ITurnBasedPlayer>& Player)
{
    const auto CurrentPlayerTurn = TurnBasedPlayers[PlayerIndexTurn];
    return CurrentPlayerTurn == Player;
}

void ATurnBasedGameState::SwitchTurn()
{
    Server_SwitchTurn();
}

void ATurnBasedGameState::SwitchTurn_Internal()
{
    const auto FormerPlayerTurn = TurnBasedPlayers[PlayerIndexTurn];
    FormerPlayerTurn->SetIsMyTurn(false);

    PlayerIndexTurn = (PlayerIndexTurn + 1) % TurnBasedPlayers.Num();
    const auto NewPlayerTurn = TurnBasedPlayers[PlayerIndexTurn];
    NewPlayerTurn->SetIsMyTurn(true);
}

void ATurnBasedGameState::BroadcastOnTurnEnd()
{
    if (GetLocalRole() < ROLE_Authority)
        Server_BroadcastOnTurnEnd();
    else
        Multicast_BroadcastOnTurnEnd();
}

void ATurnBasedGameState::Server_BroadcastOnTurnEnd_Implementation()
{
    Multicast_BroadcastOnTurnEnd();
}

bool ATurnBasedGameState::Server_BroadcastOnTurnEnd_Validate() { return true; }

void ATurnBasedGameState::Multicast_BroadcastOnTurnEnd_Implementation()
{
    BroadcastOnTurnEnd_Internal();
}

void ATurnBasedGameState::BroadcastOnTurnEnd_Internal()
{
    OnTurnEnd.Broadcast();
}
void ATurnBasedGameState::Server_SwitchTurn_Implementation()
{
    SwitchTurn_Internal();
}

bool ATurnBasedGameState::Server_SwitchTurn_Validate()
{
    return true;
}

void ATurnBasedGameState::AddPlayerState(APlayerState* PlayerState)
{
    Super::AddPlayerState(PlayerState);
    
    if (Cast<ITurnBasedPlayer>(PlayerState))
    {
        TurnBasedPlayers.AddUnique(PlayerState);
    }
}

void ATurnBasedGameState::RemovePlayerState(APlayerState* PlayerState)
{
    Super::RemovePlayerState(PlayerState);

    if (PlayerState)
    {
        for (int32 i = 0; i < TurnBasedPlayers.Num(); i++)
        {
            if (TurnBasedPlayers[i] == PlayerState)
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