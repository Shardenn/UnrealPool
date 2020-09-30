// Copyright 2019 Andrei Vikarchuk.

#include "BilliardistPawnWithPlacableBall.h"

#include "Pool.h"

#include "BilliardistController.h"
#include "GameplayLogic/PSWithHandableBall.h"
#include "Objects/Ball.h"
#include "GameplayLogic/Interfaces/PlayerWithHandableBall.h"
#include "Objects/Table/Components/InitialBallPlacementArea.h"

void ABilliardistPawnWithPlacableBall::BeginPlay()
{
    Super::BeginPlay();
    SetActorTickEnabled(true);
}

void ABilliardistPawnWithPlacableBall::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();
    SubscribeToBallInHandUpdate();

    MyController = Cast<ABilliardistController>(GetController());
    if (!ensure(MyController != nullptr)) return;
}

void ABilliardistPawnWithPlacableBall::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
    SubscribeToBallInHandUpdate();

    MyController = Cast<ABilliardistController>(GetController());
    if (!ensure(MyController != nullptr)) return;
}

void ABilliardistPawnWithPlacableBall::SubscribeToBallInHandUpdate()
{
    const auto MyPlayerState = GetPlayerState();
    const auto CastedTo = Cast<APSWithHandableBall>(MyPlayerState);
    
    if (!ensure(CastedTo != nullptr)) return;
    HandablePlayer = CastedTo;
}

void ABilliardistPawnWithPlacableBall::SetBallInHand(ABall* Ball, bool bInitialPlacementIn)
{
    OnBallInHandUpdate(Ball, bInitialPlacementIn);
}

// Gets called on server only. It features SpawnActor
void ABilliardistPawnWithPlacableBall::OnBallInHandUpdate(ABall* Ball, bool bInitialPlacementIn)
{
    if (!HandablePlayer) return;

    if (HandablePlayer->GetIsBallInHand())
    {
        if (!IsValid(GhostBallClass))
        {
            UE_LOG(LogTemp, Warning, TEXT("Ghost ball class is not set in Pawn"));
            return;
        }
        GhostHandedBall = GetWorld()->SpawnActor<ABall>(GhostBallClass, FVector(100, 0, 2000), FRotator::ZeroRotator);
        auto BallRootComp = Cast<UPrimitiveComponent>(GhostHandedBall->GetRootComponent());
        BallRootComp->SetSimulatePhysics(false);
        BallRootComp->SetCollisionResponseToAllChannels(ECR_Overlap);

        bInitialPlacement = bInitialPlacementIn;

        MyController = Cast<ABilliardistController>(GetController());
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

void ABilliardistPawnWithPlacableBall::Server_SetGhostBallLocation_Implementation(const FVector& NewLocationIn)
{
    GhostHandedBall->SetActorLocation(NewLocationIn);
}

bool ABilliardistPawnWithPlacableBall::Server_SetGhostBallLocation_Validate(const FVector& NewLocaiton)
{
    return true; // TODO maybe cheat protection here
}

void ABilliardistPawnWithPlacableBall::Client_UpdateGhostBallLocation_Implementation()
{
    FVector TableHitResult;
    if (GhostHandedBall && MyController &&
        MyController->TryRaycastTable(TableHitResult))
    {
        float BallRadius = GhostHandedBall->GetRootComponent()->Bounds.SphereRadius;
        const FVector NewLocation = TableHitResult + FVector(0, 0, BallRadius + 1);
        
        GhostHandedBall->SetActorLocation(NewLocation);
        //Server_SetGhostBallLocation(NewLocation);
        
        if (IsBallPlacementValid())
        {
            LastSuccessfullGhostBallLocation = NewLocation;
        }
        else
        {
            GhostHandedBall->SetActorLocation(LastSuccessfullGhostBallLocation);
        }
        
    }
}

void ABilliardistPawnWithPlacableBall::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Draw cue ball possible location when putting it from hand
    Client_UpdateGhostBallLocation();
}

void ABilliardistPawnWithPlacableBall::TryPlaceBall(const TScriptInterface<IPlayerWithHandableBall>& Player)
{
    Player->PlaceHandedBall(GhostHandedBall->GetActorLocation());
}

bool ABilliardistPawnWithPlacableBall::IsBallPlacementValid()
{
    if (!GhostHandedBall) return false;

    TArray<UPrimitiveComponent*> OverlappingComponents;
    const auto PrimComp = Cast<UPrimitiveComponent>(GhostHandedBall->GetRootComponent());
    PrimComp->GetOverlappingComponents(OverlappingComponents);

    bool bOverlappingStaticMesh = false;
    bool bOverlappingInitialArea = false;
    for (const auto& Component : OverlappingComponents)
    {
        if (Cast<UStaticMeshComponent>(Component) &&
            Component->GetCollisionObjectType() != ECC_PlacementAreaHint)
            bOverlappingStaticMesh = true;
        if (Cast<UInitialBallPlacementArea>(Component))
            bOverlappingInitialArea = true;
    }

    if (bInitialPlacement && !bOverlappingInitialArea ||
        bOverlappingStaticMesh)
        return false;

    return true;
}

void ABilliardistPawnWithPlacableBall::ActionReleaseHandle()
{
    // If we have a ball in hand - try place it
    if (HandablePlayer && HandablePlayer->GetIsBallInHand())
    {
        TryPlaceBall(HandablePlayer);
        bBallPlacedJustNow = true;
        return;
    }
    Super::ActionReleaseHandle();
}

void ABilliardistPawnWithPlacableBall::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ABilliardistPawnWithPlacableBall, GhostHandedBall);
    DOREPLIFETIME(ABilliardistPawnWithPlacableBall, bInitialPlacement);
}

