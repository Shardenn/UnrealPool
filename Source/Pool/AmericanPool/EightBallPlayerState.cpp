// Copyright 2019 Andrei Vikarchuk.

#include "EightBallPlayerState.h"

#include "Pool.h"

#include "GameplayLogic/Interfaces/GSWithNamedShot.h"
#include "GameplayLogic/Interfaces/GameWithMainCueBall.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"

void AEightBallPlayerState::NameShot(UPocketArea* SelectedPocket, ABall* SelectedBall)
{
    auto GS = Cast<IGSWithNamedShot>(UGameplayStatics::GetGameState(GetWorld()));
    if (!ensure(GS != nullptr)) return;
    GS->RegisterNamedShot(SelectedPocket, SelectedBall);
}

void AEightBallPlayerState::OnFrameRestarted_Internal()
{
    Super::OnFrameRestarted_Internal();
    CueBall = nullptr;
}

ABall* AEightBallPlayerState::GetCueBall()
{
    const auto GS = Cast<IGameWithMainCueBall>(UGameplayStatics::GetGameState(GetWorld()));
    return GS->GetCueBall();
}

void AEightBallPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AEightBallPlayerState, AssignedBallType);
    DOREPLIFETIME(AEightBallPlayerState, CueBall);
}