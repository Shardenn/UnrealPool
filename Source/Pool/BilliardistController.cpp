// Fill out your copyright notice in the Description page of Project Settings.

#include "BilliardistController.h"
#include "Pool.h"
#include "Billiardist.h"
#include "Objects/Ball.h"

#include "Kismet/GameplayStatics.h"

#include "DrawDebugHelpers.h"

ABilliardistController::ABilliardistController()
{

}

void ABilliardistController::BeginPlay()
{
    Super::BeginPlay();
}

void ABilliardistController::SubscribeToPlayerStateChange(ABilliardist* Billiardist)
{
    Server_SubscribeToStateChange(Billiardist);
}

bool ABilliardistController::Server_SubscribeToStateChange_Validate(ABilliardist* Billiardist) { return true; }
void ABilliardistController::Server_SubscribeToStateChange_Implementation(ABilliardist* Billiardist)
{
    Billiardist->OnStateChange.AddDynamic(this, &ABilliardistController::OnPlayerStateChanged);
}

void ABilliardistController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ABilliardistController::OnPlayerStateChanged(FBilliardistState newState, FBilliardistState OldState)
{
    OnPlayerStateChangedEvent(newState, OldState);
}

bool ABilliardistController::TryRaycastBall(ABall*& FoundBall)
{
    int32 ViewportSizeX, ViewportSizeY;
    GetViewportSize(ViewportSizeX, ViewportSizeY);
    auto ScreenLocation = FVector2D(ViewportSizeX * CrosshairXLocation, ViewportSizeY * CrosshairYLocation);

    FVector Direction; // look direction
    if (!GetLookDirection(ScreenLocation, Direction))
    {
        UE_LOG(LogPool, Error, TEXT("%s could not get LookDirection."), *GetName());
        return false;
    }

    // TODO split in the other method later
    FHitResult HitResult;
    auto StartLocation = PlayerCameraManager->GetCameraLocation();
    auto EndLocation = StartLocation + Direction * RaycastLength;
    
    if (!GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECollisionChannel::ECC_GameTraceChannel1
    ))
    {
        return false;
    }

    auto HittedActor = Cast<ABall>(HitResult.Actor);
    if (!HittedActor)
    {
        return false;
    }
    
    FoundBall = HittedActor;
    return true;
}

// TODO refactor this and function above. They are copy-paste of each other
bool ABilliardistController::TryRaycastTable(FVector& RaycastHit)
{
    int32 ViewportSizeX, ViewportSizeY;
    GetViewportSize(ViewportSizeX, ViewportSizeY);
    auto ScreenLocation = FVector2D(ViewportSizeX * CrosshairXLocation, ViewportSizeY * CrosshairYLocation);

    FVector Direction; // look direction
    if (!GetLookDirection(ScreenLocation, Direction))
    {
        return false;
    }

    // TODO split in the other method later
    FHitResult HitResult;
    auto StartLocation = PlayerCameraManager->GetCameraLocation();
    auto EndLocation = StartLocation + Direction * RaycastLength;

    if (!GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECollisionChannel::ECC_GameTraceChannel2
    ))
    {
        return false;
    }
    
    if (!HitResult.ImpactNormal.Equals(FVector::UpVector, 0.1))
    {
        UE_LOG(LogPool, Warning, TEXT("Hit result on table is not up vectored"));
        return false;
    }
    RaycastHit = HitResult.Location;
    return true;
}

bool ABilliardistController::GetLookDirection(FVector2D ScreenLocation, FVector & LookDirection) const
{
    FVector CameraWorldLocation; // to de discarded
    return DeprojectScreenPositionToWorld(
        ScreenLocation.X,
        ScreenLocation.Y,
        CameraWorldLocation,
        LookDirection
    );
}


void ABilliardistController::SetExaminingView()
{
    
}

void ABilliardistController::ReturnFromExaminingView()
{
    
}

void ABilliardistController::LookAtBall()
{
    FMinimalViewInfo cameraInfo;
    GetPawn()->CalcCamera(
        UGameplayStatics::GetWorldDeltaSeconds(GetWorld()),
        cameraInfo);
    // TODO
    /*
    auto rot = UKismetMathLibrary::FindLookAtRotation(
        cameraInfo.Location,
        m_pSelectedBall->GetActorLocation());

    // make character look at ball
    SetControlRotation(rot);
    */
}
