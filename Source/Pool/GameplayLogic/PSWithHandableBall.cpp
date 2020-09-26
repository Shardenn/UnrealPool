// Copyright 2019 Andrei Vikarchuk.

#include "PSWithHandableBall.h"
#include "Pool.h"

#include "AmericanPool/EightBallGameState.h"

#include "Objects/Ball.h"
#include "GameplayLogic/Interfaces/BallInHandUpdateListener.h"
#include "GameplayLogic/Interfaces/GameWithHandableBall.h"
#include "Player/Interfaces/BilliardistWithPlacableBall.h"
//#include "GameplayLogic/Interfaces/GameWithMainCueBall.h"
//#include "GameplayLogic/Interfaces/GSWithNamedShot.h"

#include "Kismet/GameplayStatics.h"

void APSWithHandableBall::SetIsMyTurn(const bool bInMyTurn) noexcept
{
    Super::SetIsMyTurn(bInMyTurn);
}

// Called on server from GameState
void APSWithHandableBall::SetBallInHand(ABall* Ball, bool bInitialPlacementIn)
{
    HandedBall = FHandedBallState(Ball, bInitialPlacementIn);

    UE_LOG(LogTemp, Warning, TEXT("APSWithHandableBall::SetBallInHand"));
    auto PawnWithHandableBall = Cast<IBilliardistWithPlacableBall>(GetPawn());
    PawnWithHandableBall->SetBallInHand(HandedBall.Ball, HandedBall.bInitialPlacement);
}

void APSWithHandableBall::PlaceHandedBall(const FVector& Location)
{
    Server_PlaceHandedBall(Location);
}

bool APSWithHandableBall::Server_PlaceHandedBall_Validate(const FVector&) { return true; }
void APSWithHandableBall::Server_PlaceHandedBall_Implementation(const FVector& Location)
{
    PlaceHandedBall_Internal(Location);
}

void APSWithHandableBall::PlaceHandedBall_Internal(const FVector& Location)
{
    if (!HandedBall.Ball)
    {
        UE_LOG(LogPool, Warning, TEXT("PlaceCueBall: CueBallHanded == nullptr"));
        return;
    }

    HandedBall.Ball->ReturnBallIntoGame();

    HandedBall.Ball->SetActorLocation(Location);
    TScriptInterface<IGameWithHandableBall> GameStateWithHandableBall{
        UGameplayStatics::GetGameState(GetWorld()) };
    check(GameStateWithHandableBall);

    GameStateWithHandableBall->TakeBallFromHand(this, HandedBall.Ball);
}

void APSWithHandableBall::SubscribeToBallInHandUpdate(const TScriptInterface<IBallInHandUpdateListener>& Listener)
{
    BallInHandUpdateListeners.AddUnique(Listener);
    const FString auth = HasAuthority() ? "Server" : "Client";
    UE_LOG(LogTemp, Warning, TEXT("PlayerState: subscribed on %s"), *auth);
}

void APSWithHandableBall::Multicast_BroadcastBallInHandUpdate_Implementation(ABall* Ball, bool bInitialPlacementIn)
{
    /*
    for (const auto& Listener : BallInHandUpdateListeners)
    {
        Listener->OnBallInHandUpdate(Ball, bInitialPlacementIn);
    }*/
    if (OnBallInHandUpdate.IsBound())
        OnBallInHandUpdate.Broadcast(Ball, bInitialPlacementIn);
}

void APSWithHandableBall::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APSWithHandableBall, HandedBall);
}
