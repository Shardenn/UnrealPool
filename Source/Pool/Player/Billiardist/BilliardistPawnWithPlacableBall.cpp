// Copyright 2019 Andrei Vikarchuk.

#include "BilliardistPawnWithPlacableBall.h"

#include "Pool.h"

#include "BilliardistController.h"
#include "GameplayLogic/PoolPlayerState.h"
#include "Objects/Ball.h"
#include "GameplayLogic/Interfaces/PlayerWithHandableBall.h"

void ABilliardistPawnWithPlacableBall::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Draw cue ball possible location when putting it from hand
    FVector TableHitResult;
    if (GhostHandedBall && BillController && 
        BillController->TryRaycastTable(TableHitResult))
    {
        float BallRadius = GhostHandedBall->GetRootComponent()->Bounds.SphereRadius;
        GhostHandedBall->SetActorLocation(TableHitResult + FVector(0, 0, BallRadius + 1));
    }
}

void ABilliardistPawnWithPlacableBall::TryPlaceBall(const TScriptInterface<IPlayerWithHandableBall>& Player)
{
    auto BilliardistController = Cast<ABilliardistController>(GetController());
    check(BilliardistController);

    FVector TableHitResult;
    if (IsBallPlacementValid() && 
        BilliardistController->TryRaycastTable(TableHitResult))
    {
        Player->PlaceHandedBall(TableHitResult);
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
    if (Role < ROLE_Authority)
    {
        if (!Ball)
        {
            UE_LOG(LogPool, Warning, TEXT("OnBallInHandUpdate called with nullptr"));
        }
        else
        {
            UE_LOG(LogPool, Warning, TEXT("OnBallInHandUpdate called with %s"), *Ball->GetName());
        }
    }
    if (Ball)
    {
        if (!IsValid(GhostBallClass))
        {
            UE_LOG(LogTemp, Warning, TEXT("Ghost ball class is not set"));
            return;
        }
        GhostHandedBall = GetWorld()->SpawnActor<ABall>(GhostBallClass, FVector(0, 0, 2000), FRotator::ZeroRotator);
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

void ABilliardistPawnWithPlacableBall::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    if (Role < ROLE_Authority)
    {
        UE_LOG(LogTemp, Warning, TEXT("PossesedBy called on client"));
    }

    if (Cast<IPlayerWithHandableBall>(BillPlayerState))
    {
        HandablePlayer = BillPlayerState;
        HandablePlayer->SubscribeToBallInHandUpdate(this);

        if (Role < ROLE_Authority)
        {
            UE_LOG(LogTemp, Warning, TEXT("Subscribed on client"));
        }
    }
    else
    {
        UE_LOG(LogPool, Error, TEXT("Object %s of class ABilliardistPawnWithPlacableBall has HandablePlayerState nullptr"),
            *GetName());
    }

    BillController = Cast<ABilliardistController>(GetController());
    if (!BillController)
    {
        UE_LOG(LogPool, Error, TEXT("Object %s of class ABilliardistPawnWithPlacableBall has BillController nullptr"),
            *GetName());
    }
}
