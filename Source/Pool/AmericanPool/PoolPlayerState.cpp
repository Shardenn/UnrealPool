// Fill out your copyright notice in the Description page of Project Settings.


#include "PoolPlayerState.h"
#include "AmericanPool/PoolGameState.h"

#include "UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"

void APoolPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APoolPlayerState, bIsReady);
    DOREPLIFETIME(APoolPlayerState, bMyTurn);
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

//bool APoolPlayerState::SetIsMyTurn_Validate(bool bInIsMyTurn) { return true; }
void APoolPlayerState::SetIsMyTurn(bool bInIsMyTurn)
{
    bMyTurn = bInIsMyTurn;
}
