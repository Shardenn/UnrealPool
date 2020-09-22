// Copyright 2019 Andrei Vikarchuk.

#include "BilliardistPawnWithPlacableBall.h"

#include "Pool.h"

#include "BilliardistController.h"
#include "GameplayLogic/PoolPlayerState.h"
#include "Objects/Ball.h"
#include "GameplayLogic/Interfaces/PlayerWithHandableBall.h"

void ABilliardistPawnWithPlacableBall::BeginPlay()
{
    Super::BeginPlay();
    SubscribeToBallInHandUpdate();
}

void ABilliardistPawnWithPlacableBall::SubscribeToBallInHandUpdate()
{
    const auto MyPlayerState = GetPlayerState();
    const auto CastedToInterface = Cast<IPlayerWithHandableBall>(MyPlayerState);
    if (CastedToInterface)
    {
        HandablePlayer.SetInterface(CastedToInterface);
        HandablePlayer.SetObject(MyPlayerState);
        HandablePlayer->SubscribeToBallInHandUpdate(this);

        if (GetLocalRole() < ROLE_Authority)
        {
            UE_LOG(LogTemp, Warning, TEXT("Subscribed on client"));
        }
    }
}

void ABilliardistPawnWithPlacableBall::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Draw cue ball possible location when putting it from hand
    FVector TableHitResult;
    const auto BillController = Cast<ABilliardistController>(GetController());
    if (GhostHandedBall && BillController && 
        BillController->TryRaycastTable(TableHitResult))
    {
        float BallRadius = GhostHandedBall->GetRootComponent()->Bounds.SphereRadius;
        GhostHandedBall->SetActorLocation(TableHitResult + FVector(0, 0, BallRadius + 1));
        if (IsBallPlacementValid())
        {
            PreviousGhostBallLocation = GhostHandedBall->GetActorLocation();
        }
        else
        {
            GhostHandedBall->SetActorLocation(PreviousGhostBallLocation);
        }
    }
}

void ABilliardistPawnWithPlacableBall::TryPlaceBall(const TScriptInterface<IPlayerWithHandableBall>& Player)
{
    auto BilliardistController = Cast<ABilliardistController>(GetController());
    check(BilliardistController);

    //FVector TableHitResult;
    if (IsBallPlacementValid())
        //&& BilliardistController->TryRaycastTable(TableHitResult))
    {
        Player->PlaceHandedBall(GhostHandedBall->GetActorLocation());
    }
}

bool ABilliardistPawnWithPlacableBall::IsBallPlacementValid()
{
    if (!GhostHandedBall) return false;

    TArray<UPrimitiveComponent*> OverlappingComponents;
    const auto PrimComp = Cast<UPrimitiveComponent>(GhostHandedBall->GetRootComponent());
    PrimComp->GetOverlappingComponents(OverlappingComponents);

    for (const auto& Component : OverlappingComponents)
    {
        if (Cast<UStaticMeshComponent>(Component))
        {
            UE_LOG(LogPool, Warning, TEXT("Overlapping with %s, cannot place cue ball"), *Component->GetName());
            return false;
        }
    }

    return true;
}

void ABilliardistPawnWithPlacableBall::OnBallInHandUpdate(class ABall* const Ball)
{
    if (Ball)
    {
        if (!IsValid(GhostBallClass))
        {
            UE_LOG(LogTemp, Warning, TEXT("Ghost ball class is not set"));
            return;
        }
        GhostHandedBall = GetWorld()->SpawnActor<ABall>(GhostBallClass, FVector(100, 0, 2000), FRotator::ZeroRotator);
        auto BallRootComp = Cast<UPrimitiveComponent>(GhostHandedBall->GetRootComponent());
        BallRootComp->SetSimulatePhysics(false);
        BallRootComp->SetCollisionResponseToAllChannels(ECR_Overlap);
    }
    else
    {
        if (GhostHandedBall)
        {
            GhostHandedBall->Destroy();
            GhostHandedBall = nullptr;
        }
    }
}

void ABilliardistPawnWithPlacableBall::ActionReleaseHandle()
{    
    // If we have a ball in hand - try place it
    if (HandablePlayer && HandablePlayer->GetIsBallInHand())
    {
        TryPlaceBall(HandablePlayer);
        return;
    }
    Super::ActionReleaseHandle();
}
