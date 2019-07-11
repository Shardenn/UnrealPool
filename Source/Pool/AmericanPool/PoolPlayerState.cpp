// Fill out your copyright notice in the Description page of Project Settings.


#include "PoolPlayerState.h"
#include "AmericanPool/PoolGameState.h"

#include "UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"

void APoolPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APoolPlayerState, bIsReady);
}

bool APoolPlayerState::ToggleReady_Validate() { return true; }
void APoolPlayerState::ToggleReady_Implementation()
{
    bIsReady = !bIsReady;

    APoolGameState* GameState = Cast<APoolGameState>(UGameplayStatics::GetGameState(GetWorld()));
    if (!ensure(GameState != nullptr)) return;

    uint32 Offset = bIsReady ? 1 : -1;
    GameState->SetPlayersReadyNum(GameState->PlayersReadyNum + Offset);
}

bool APoolPlayerState::IsMyTurn()
{
    APoolGameState* GameState = Cast<APoolGameState>(UGameplayStatics::GetGameState(GetWorld()));
    if (!ensure(GameState != nullptr)) return false;

    if (GameState->RequestIsPlayerTurn(this))
        return true;

    return false;
}
