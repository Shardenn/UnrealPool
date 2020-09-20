// Copyright 2019 Andrei Vikarchuk.

#include "EightBallPlayerState.h"
#include "Pool.h"

#include "EightBallGameState.h"

#include "Objects/Ball.h"
#include "Player/Billiardist/BilliardistWithPlacableBall.h"
#include "GameplayLogic/Interfaces/BallInHandUpdateListener.h"
#include "GameplayLogic/Interfaces/GameWithHandableBall.h"
#include "GameplayLogic/Interfaces/GameWithMainCueBall.h"

#include "Kismet/GameplayStatics.h"

void AEightBallPlayerState::SetIsMyTurn(const bool bInMyTurn) noexcept
{
    Super::SetIsMyTurn(bInMyTurn);
}

void AEightBallPlayerState::SetBallInHand(ABall* const Ball)
{
    BallHanded = Ball;
    Multicast_BroadcastBallInHandUpdate(Ball);
}

void AEightBallPlayerState::PlaceHandedBall(const FVector& TablePoint)
{
    Server_PlaceHandedBall(TablePoint);
}

ABall* AEightBallPlayerState::GetCueBall()
{
    const auto GS = Cast<IGameWithMainCueBall>(UGameplayStatics::GetGameState(GetWorld()));
    return GS->GetCueBall();
}

bool AEightBallPlayerState::Server_PlaceHandedBall_Validate(const FVector&) { return true; }
void AEightBallPlayerState::Server_PlaceHandedBall_Implementation(const FVector& TablePoint)
{
    PlaceHandedBall_Internal(TablePoint);
}

void AEightBallPlayerState::PlaceHandedBall_Internal(const FVector& TablePoint)
{
    if (!BallHanded)
    {
        UE_LOG(LogPool, Warning, TEXT("PlaceCueBall: CueBallHanded == nullptr"));
        return;
    }

    UWorld* World = GetWorld();
    if (!World) return;

    BallHanded->ReturnBallIntoGame();

    float BallRadius = BallHanded->GetRootComponent()->Bounds.SphereRadius;
    BallHanded->SetActorLocation(TablePoint + FVector(0, 0, BallRadius + 1));

    TScriptInterface<IGameWithHandableBall> GameStateWithHandableBall{
        UGameplayStatics::GetGameState(World) };
    check(GameStateWithHandableBall);

    GameStateWithHandableBall->TakeBallFromHand(this, BallHanded);
}

void AEightBallPlayerState::OnFrameRestarted_Internal()
{
    Super::OnFrameRestarted_Internal();
    CueBall = nullptr;
}

void AEightBallPlayerState::SubscribeToBallInHandUpdate(const TScriptInterface<IBallInHandUpdateListener>& Listener)
{
    BallInHandUpdateListeners.AddUnique(Listener);
}

void AEightBallPlayerState::Multicast_BroadcastBallInHandUpdate_Implementation(ABall* Ball)
{
    for (const auto& Listener : BallInHandUpdateListeners)
    {
        Listener->OnBallInHandUpdate(Ball);
    }
}

void AEightBallPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AEightBallPlayerState, BallHanded);
    DOREPLIFETIME(AEightBallPlayerState, AssignedBallType);
}
