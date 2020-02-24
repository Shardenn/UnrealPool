// Copyright 2019 Andrei Vikarchuk.

#include "PoolPlayerState.h"
#include "Pool.h"

#include "GameplayLogic/PoolGameState.h"
#include "Player/Billiardist/BilliardistPawn.h" // TODO dependency invertion is violated

#include "Kismet/GameplayStatics.h"

bool APoolPlayerState::Server_ToggleReady_Validate() { return true; }
void APoolPlayerState::Server_ToggleReady_Implementation()
{
    bIsReady = !bIsReady;

    APoolGameState* GameState = Cast<APoolGameState>(UGameplayStatics::GetGameState(GetWorld()));
    if (!ensure(GameState != nullptr)) return;

    uint32 Offset = bIsReady ? 1 : -1;
    GameState->SetPlayersReadyNum(GameState->PlayersReadyNum + Offset);
}

//bool APoolPlayerState::SetIsMyTurn_Validate(bool bInIsMyTurn) { return true; }
void APoolPlayerState::SetIsMyTurn(const bool bInIsMyTurn) noexcept
{
    bMyTurn = bInIsMyTurn;
    Cast<ABilliardistPawn>(GetPawn())->NotifyTurnUpdate(bMyTurn);
} 

bool APoolPlayerState::Server_HandleFrameWon_Validate() { return true; }
void APoolPlayerState::Server_HandleFrameWon_Implementation()
{
    FramesWon++;
}

void APoolPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APoolPlayerState, bIsReady);
    DOREPLIFETIME(APoolPlayerState, bMyTurn);
    DOREPLIFETIME(APoolPlayerState, FramesWon);
}
