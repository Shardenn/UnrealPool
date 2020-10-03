// Copyright 2019 Andrei Vikarchuk.

#include "EightBallPlayerState.h"

#include "Pool.h"

#include "GameplayLogic/Interfaces/GSWithNamedShot.h"
#include "GameplayLogic/Interfaces/GameWithMainCueBall.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"

void AEightBallPlayerState::NameShot(UPocketArea* SelectedPocket, ABall* SelectedBall)
{
    Server_NameShot(SelectedPocket, SelectedBall);
}

void AEightBallPlayerState::Server_NameShot_Implementation(UPocketArea* SelectedPocket, ABall* SelectedBall)
{
    NameShot_Internal(SelectedPocket, SelectedBall);
}

void AEightBallPlayerState::NameShot_Internal(UPocketArea* SelectedPocket, ABall* SelectedBall)
{
    auto GS = Cast<IGSWithNamedShot>(UGameplayStatics::GetGameState(GetWorld()));
    if (!ensure(GS != nullptr)) return;
    GS->RegisterNamedShot(SelectedPocket, SelectedBall);
}

void AEightBallPlayerState::PredictShot(UPocketArea* SelectedPocket, ABall* SelectedBall)
{
    Server_PredictShot(SelectedPocket, SelectedBall);
}

void AEightBallPlayerState::Server_PredictShot_Implementation(UPocketArea* SelectedPocket, ABall* SelectedBall)
{
    PredictShot_Internal(SelectedPocket, SelectedBall);
}

void AEightBallPlayerState::PredictShot_Internal(UPocketArea* SelectedPocket, ABall* SelectedBall)
{
    auto GS = Cast<IGSWithNamedShot>(UGameplayStatics::GetGameState(GetWorld()));
    if (!ensure(GS != nullptr)) return;
    GS->RegisterPredictedShot(SelectedPocket, SelectedBall);
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

bool AEightBallPlayerState::Server_NameShot_Validate(UPocketArea* SelectedPocket, ABall* SelectedBall) { return true; }

bool AEightBallPlayerState::Server_PredictShot_Validate(UPocketArea* SelectedPocket, ABall* SelectedBall) { return true; }

void AEightBallPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AEightBallPlayerState, AssignedBallType);
    DOREPLIFETIME(AEightBallPlayerState, CueBall);
}