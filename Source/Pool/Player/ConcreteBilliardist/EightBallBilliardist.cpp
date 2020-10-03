// Copyright 2019 Andrei Vikarchuk.

#include "EightBallBilliardist.h"

#include "Pool.h"

#include "GameplayLogic/Interfaces/PlayerWithMainCueBall.h"
#include "GameplayLogic/Interfaces/PlayerWithNamedShot.h"

#include "Objects/Table/Components/PocketArea.h"
#include "BilliardistController.h"

#include "Objects/Ball.h"
#include "Objects/BallAmerican.h"

#include "GameFramework/PlayerState.h"
#include "DrawDebugHelpers.h"

//#define DRAW_DEBUG 

void AEightBallBilliardist::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    bool bInControlOfPawn = GetLocalRole() == ROLE_AutonomousProxy ||
        GetRemoteRole() == ROLE_SimulatedProxy;
    
    if (State == FBilliardistState::AIMING && bInControlOfPawn && !SelectedPocket)
    {
        if (!SelectedBall)
        {
            UE_LOG(LogTemp, Warning, TEXT("%s: no SelectedBall in Tick."), *GetName());
            return;
        }
        const FVector BallLocation = SelectedBall->GetActorLocation();
        const float BallRadius = SelectedBall->GetRootComponent()->Bounds.SphereRadius;

        if (!CamManager)
        {
            UE_LOG(LogTemp, Warning, TEXT("%s: no CamManager in Tick"), *GetName());
            return;
        }
        FVector LookDir = CamManager->GetCameraRotation().Vector();
        LookDir.Z = 0.f;
        
        const FVector StartLocation = BallLocation + LookDir * BallRadius;
        
        FVector ImpactPoint;
        ABall* PredictedBall = GetBallInLineOfSight(StartLocation, LookDir, BallRadius, ImpactPoint);
        ABallAmerican* BallAm = Cast<ABallAmerican>(PredictedBall);
        if (!BallAm) return;

        ImpactPoint.Z = BallAm->GetActorLocation().Z;

        FVector BallRollDirection = BallAm->GetActorLocation() - ImpactPoint;
        BallRollDirection.Normalize();

        float PredictedBallRadius = BallAm->GetRootComponent()->Bounds.SphereRadius;
        FVector PocketTraceStart = BallAm->GetActorLocation() + BallRollDirection * PredictedBallRadius * 1.01;

        UPocketArea* PredictedPocket = GetPocketInLineOfShot(PocketTraceStart, BallRollDirection);
        if (PredictedPocket)
        {
            LastPredictedBall = BallAm;
            PredictedPocket->OnHoverOver();
            LastPredictedPocket = PredictedPocket;
        }
        // if we lost the previous pocket, tell it to stop glowing
        else if (LastPredictedPocket != nullptr && 
            (!PredictedPocket || PredictedPocket != LastPredictedPocket))
        {
            LastPredictedPocket->OnStopHoverOver();
            LastPredictedPocket = nullptr;
            LastPredictedBall = nullptr;
        }
    }
}

void AEightBallBilliardist::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();
    CamManager = GetWorld()->GetFirstPlayerController()->PlayerCameraManager;
}

void AEightBallBilliardist::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
    CamManager = GetWorld()->GetFirstPlayerController()->PlayerCameraManager;
}

void AEightBallBilliardist::NameShot(UPocketArea* InSelectedPocket, ABall* InSelectedBall)
{
    TScriptInterface<IPlayerWithNamedShot> PlayerWithNamed = GetPlayerState();
    if (!ensure(PlayerWithNamed != nullptr)) return;

    PlayerWithNamed->NameShot(InSelectedPocket, InSelectedBall);
}

void AEightBallBilliardist::PredictShot(UPocketArea* InSelectedPocket, ABall* InSelectedBall)
{
    TScriptInterface<IPlayerWithNamedShot> PlayerWithNamed = GetPlayerState();
    if (!ensure(PlayerWithNamed != nullptr)) return;

    PlayerWithNamed->PredictShot(InSelectedPocket, InSelectedBall);
}

void AEightBallBilliardist::ActionPressHandle()
{
    Super::ActionPressHandle();

    
}

void AEightBallBilliardist::ActionReleaseHandle()
{
    Super::ActionReleaseHandle();

    using FState = FBilliardistState;

    const auto BillController = Cast<ABilliardistController>(GetController());
    if (!ensure(BillController != nullptr)) return;

    switch (State)
    {
    case FState::NAMING_SHOT:
    {
        auto RaycastedBall = BillController->TryRaycastBall();
        if (RaycastedBall)
        {
            NameShot(SelectedPocket, RaycastedBall);
            SelectedPocket->OnSelected();
            SetState(FState::WALKING);
        }
        break;
    }
    case FState::WALKING:
    {
        if (bBallPlacedJustNow)
        {
            bBallPlacedJustNow = false;
            break;
        }

        SelectedPocket = BillController->TryRaycastPocketArea();
        if (SelectedPocket)
        {
            SetState(FState::NAMING_SHOT);
            SelectedPocket->OnSelected();
            break;
        }

        TScriptInterface<IPlayerWithMainCueBall> PlayerWithCueBall = GetPlayerState();
        if (!ensure(PlayerWithCueBall != nullptr)) return;

        SelectedBall = PlayerWithCueBall->GetCueBall();

        if (SelectedBall)
            HandleBallSelected(SelectedBall);
        else
            UE_LOG(LogTemp, Warning, TEXT("BilliardistPawn::ActionReleaseHandle: SelectedBall is nullptr"));

        break;
    }
    case FState::OBSERVING:
    {
        PredictShot(LastPredictedPocket, LastPredictedBall);
        if (LastPredictedPocket)
        {
            LastPredictedPocket->OnStopHoverOver();
            LastPredictedPocket->OnSelected();
            LastPredictedPocket = nullptr;
        }
        LastPredictedBall = nullptr;
    }

    }
}

void AEightBallBilliardist::DrawPredictedLine()
{
    FVector LookDir = CamManager->GetCameraRotation().Vector();
    //LookDir = LookDir.Normalize();
    LookDir.Z = 0.f;

    float BallRadius = SelectedBall->GetRootComponent()->Bounds.SphereRadius * 1.01;
    FVector TraceStart = SelectedBall->GetActorLocation() + LookDir * BallRadius;

    float CastLength = 300;
    FVector TraceEnd = TraceStart + LookDir * CastLength;

#ifdef DRAW_DEBUG
    DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, 2);
#endif
}

ABall* AEightBallBilliardist::GetBallInLineOfSight(const FVector& StartLocation, const FVector& LookDirection, 
    const float BallRadius, FVector& ImpactPoint)
{
    FHitResult HitRes;
    const FVector EndLocation = StartLocation + LookDirection * PredictedShotCastLength;

    FCollisionShape SphereShape = FCollisionShape::MakeSphere(BallRadius);
    GetWorld()->SweepSingleByChannel(HitRes, StartLocation, EndLocation, FQuat::Identity, 
        ECollisionChannel::ECC_BallTraceChannel, SphereShape);
    ImpactPoint = HitRes.ImpactPoint;

    return Cast<ABall>(HitRes.Actor);
}

UPocketArea* AEightBallBilliardist::GetPocketInLineOfShot(const FVector& StartLocation, const FVector& RollDirection)
{
    FHitResult HitRes;
    for (int i = 0; i < PredictedPocketConeAngle * 0.5 / PredictedPocketConeStep; i++)
    {
        FVector DirectionTry;
        FVector EndLocation;

        DirectionTry = RollDirection.RotateAngleAxis(PredictedPocketConeStep * i, FVector(0, 0, 1));
        EndLocation = StartLocation + DirectionTry * PredictedShotCastLength;
        if (GetWorld()->LineTraceSingleByChannel(HitRes, StartLocation, EndLocation, ECollisionChannel::ECC_PocketAreaTracing))
            return Cast<UPocketArea>(HitRes.Component);
#ifdef DRAW_DEBUG
        DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Green);
#endif
        DirectionTry = RollDirection.RotateAngleAxis(PredictedPocketConeStep * (-i), FVector(0, 0, 1));
        EndLocation = StartLocation + DirectionTry * PredictedShotCastLength;
        if (GetWorld()->LineTraceSingleByChannel(HitRes, StartLocation, EndLocation, ECollisionChannel::ECC_PocketAreaTracing))
            return Cast<UPocketArea>(HitRes.Component);
#ifdef DRAW_DEBUG
        DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Green);
#endif
    }
    return nullptr;
}
