// Copyright 2019 Andrei Vikarchuk.

#include "EightBallPlayerState.h"
#include "Pool.h"

#include "EightBallGameState.h"

#include "Objects/Ball.h"

#include "Kismet/GameplayStatics.h"

// TODO refactor
#include "Player/Billiardist/BilliardistPawn.h"

void AEightBallPlayerState::SetBallInHand(ABall* const CueBall)
{
    BallHanded = CueBall;
    // TODO refactor
    Cast<ABilliardistPawn>(GetPawn())->Client_NotifyBallInHand(CueBall != nullptr);
}

void AEightBallPlayerState::PlaceHandedBall(const FVector& TablePoint) const
{
    Server_PlaceHandedBall(TablePoint);
}

bool AEightBallPlayerState::Server_PlaceHandedBall_Validate(const FVector&) { return true; }
void AEightBallPlayerState::Server_PlaceHandedBall_Implementation(const FVector& TablePoint) const
{
    PlaceHandedBall_Internal(TablePoint);
}

void AEightBallPlayerState::PlaceHandedBall_Internal(const FVector& TablePoint) const
{
    if (!BallHanded)
    {
        UE_LOG(LogPool, Warning, TEXT("PlaceCueBall: CueBallHanded == nullptr"));
        return;
    }

    UWorld* World = GetWorld();
    if (!World) return;

    float BallRadius = BallHanded->GetRootComponent()->Bounds.SphereRadius;

    // If we first SetLocation and then SimulatePhys(true), then SetLocation not working
    Cast<UPrimitiveComponent>(BallHanded->GetRootComponent())->SetSimulatePhysics(true);
    BallHanded->SetActorLocation(TablePoint + FVector(0, 0, BallRadius + 1));

    const auto State = Cast<IGameWithHandableBall>(UGameplayStatics::GetGameState(World));
    check(State);

   // auto Me = Cast<IPlayerWithHandableBall>(this);
    //State->TakeBallFromHand(this, BallHanded);
}

void AEightBallPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AEightBallPlayerState, BallHanded);
    DOREPLIFETIME(AEightBallPlayerState, AssignedBallType);
}
