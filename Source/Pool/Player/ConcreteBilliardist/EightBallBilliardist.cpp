// Copyright 2019 Andrei Vikarchuk.

#include "EightBallBilliardist.h"

#include "GameplayLogic/Interfaces/PlayerWithMainCueBall.h"
#include "GameplayLogic/Interfaces/PlayerWithNamedShot.h"

#include "Objects/Table/Components/PocketArea.h"
#include "BilliardistController.h"

#include "GameFramework/PlayerState.h"

void AEightBallBilliardist::NameShot(UPocketArea* InSelectedPocket, ABall* InSelectedBall)
{
    TScriptInterface<IPlayerWithNamedShot> PlayerWithNamed = GetPlayerState();
    if (!ensure(PlayerWithNamed != nullptr)) return;

    PlayerWithNamed->NameShot(InSelectedPocket, InSelectedBall);
}

void AEightBallBilliardist::ActionReleaseHandle()
{
    Super::ActionReleaseHandle();

    using FState = FBilliardistState;

    const auto BillController = Cast<ABilliardistController>(GetController());
    if (!ensure(BillController != nullptr)) return;

    SelectedPocket = BillController->TryRaycastPocketArea();
    if (SelectedPocket)
    {
        SetState(FState::NAMING_SHOT);
        SelectedPocket->OnSelected();
    }

    switch (State)
    {
    case FState::NAMING_SHOT:
    {
        auto RaycastedBall = BillController->TryRaycastBall();
        if (RaycastedBall)
        {
            NameShot(SelectedPocket, RaycastedBall);
            SetState(FState::WALKING);
        }
        break;
    }
    case FState::WALKING:
    {
        TScriptInterface<IPlayerWithMainCueBall> PlayerWithCueBall = GetPlayerState();
        if (!ensure(PlayerWithCueBall != nullptr)) return;

        SelectedBall = PlayerWithCueBall->GetCueBall();
     
        if (SelectedBall)
            HandleBallSelected(SelectedBall);
        else
            UE_LOG(LogTemp, Warning, TEXT("BilliardistPawn::ActionReleaseHandle: SelectedBall is nullptr"));

        break;
    }
    }
}
