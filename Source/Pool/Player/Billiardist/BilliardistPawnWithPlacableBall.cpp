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
}

void ABilliardistPawnWithPlacableBall::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
    SubscribeToBallInHandUpdate();
}

void ABilliardistPawnWithPlacableBall::SubscribeToBallInHandUpdate()
{
    const auto MyPlayerState = GetPlayerState();
    const auto CastedTo = Cast<APSWithHandableBall>(MyPlayerState);
    if (CastedTo)
    {
        UE_LOG(LogTemp, Warning, TEXT("ABilliardistPawnWithPlacableBall::SubscribeToBallInHandUpdate casted successfully"));
        //CastedTo->OnBallInHandUpdate.AddDynamic(this, &ABilliardistPawnWithPlacableBall::OnBallInHandUpdate);

        HandablePlayer = CastedTo;
    }
}

void ABilliardistPawnWithPlacableBall::SetBallInHand(ABall* Ball, bool bInitialPlacementIn)
{
    const FString ballNull = Ball == nullptr ? "null" : Ball->GetName();
    UE_LOG(LogTemp, Warning, TEXT("ABilliardistPawnWithPlacableBall::SetBallInHand: %s"), *ballNull);
    const FString auth = HasAuthority() ? "server" : "client";
    UE_LOG(LogTemp, Warning, TEXT("I am %s version of %s"), *auth, *GetName());

    //Client_OnBallInHandUpdate(Ball, bInitialPlacementIn);
    OnBallInHandUpdate(Ball, bInitialPlacementIn);
}

void ABilliardistPawnWithPlacableBall::Client_OnBallInHandUpdate_Implementation(ABall* Ball, bool bInitialPlacementIn /*= false*/)
{
    const FString ballNull = Ball == nullptr ? "null" : Ball->GetName();
    UE_LOG(LogTemp, Warning, TEXT("ABilliardistPawnWithPlacableBall::Client_OnBallInHandUpdate_Implementation: %s"), *ballNull);
    const FString auth = HasAuthority() ? "server" : "client";
    UE_LOG(LogTemp, Warning, TEXT("I am %s version of %s"), *auth, *GetName());
    OnBallInHandUpdate(Ball, bInitialPlacementIn);
}

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

        bInitialPlacement = HandablePlayer->GetIsInitialPlacement();
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

    if (IsBallPlacementValid())
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
        return;
    }
    Super::ActionReleaseHandle();
}

void ABilliardistPawnWithPlacableBall::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ABilliardistPawnWithPlacableBall, GhostHandedBall);
}

