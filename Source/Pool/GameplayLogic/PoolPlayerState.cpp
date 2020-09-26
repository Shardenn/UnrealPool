// Copyright 2019 Andrei Vikarchuk.

#include "PoolPlayerState.h"
#include "Pool.h"

#include "GameplayLogic/PoolGameMode.h"
#include "GameplayLogic/PoolGameState.h"

#include "Kismet/GameplayStatics.h"

void APoolPlayerState::BeginPlay()
{
    Super::BeginPlay();

    SetReplicates(true);

    if (HasAuthority())
    {
        APoolGameMode* GM = Cast<APoolGameMode>(GetWorld()->GetAuthGameMode());
        if (!ensure(GM != nullptr)) return;

        GM->OnFrameRestart.AddDynamic(this, &APoolPlayerState::Client_OnFrameRestarted);
    }
} 

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
} 

void APoolPlayerState::Client_OnFrameRestarted_Implementation()
{
    OnFrameRestarted_Internal();
}

void APoolPlayerState::OnFrameRestarted_Internal()
{
    if (OnFrameRestartedDelegate.IsBound())
        OnFrameRestartedDelegate.Broadcast();
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
