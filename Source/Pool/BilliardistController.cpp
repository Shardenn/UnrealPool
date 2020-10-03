// Copyright 2019 Andrei Vikarchuk.

#include "BilliardistController.h"
#include "Pool.h"
#include "Player/Billiardist/BilliardistPawn.h"
#include "Objects/Ball.h"
#include "Objects/Table/Components/PocketArea.h"

#include "Kismet/GameplayStatics.h"

#include "DrawDebugHelpers.h"

#include <chrono>

ABilliardistController::ABilliardistController()
{

}

void ABilliardistController::BeginPlay()
{
    Super::BeginPlay();

    if (GetLocalRole() < ROLE_Authority)
    {
        ServerTimeRequestWasPlaced = GetLocalTime();
        Server_GetServerTime();
    }
}

void ABilliardistController::SubscribeToPlayerStateChange(ABilliardistPawn* Billiardist)
{
    //Server_SubscribeToStateChange(Billiardist);
    Billiardist->OnStateChange.AddDynamic(this, &ABilliardistController::OnPlayerStateChanged);
}

bool ABilliardistController::Server_SubscribeToStateChange_Validate(ABilliardistPawn* Billiardist) { return true; }
void ABilliardistController::Server_SubscribeToStateChange_Implementation(ABilliardistPawn* Billiardist)
{
    Billiardist->OnStateChange.AddDynamic(this, &ABilliardistController::OnPlayerStateChanged);
}

void ABilliardistController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ABilliardistController::OnPlayerStateChanged(FBilliardistState newState)
{
    OnPlayerStateChangedEvent(newState);
}

ABall* ABilliardistController::TryRaycastBall()
{
    int32 ViewportSizeX, ViewportSizeY;
    GetViewportSize(ViewportSizeX, ViewportSizeY);
    auto ScreenLocation = FVector2D(ViewportSizeX * CrosshairXLocation, ViewportSizeY * CrosshairYLocation);

    FVector Direction; // look direction
    if (!GetLookDirection(ScreenLocation, Direction)) return nullptr; 

    // TODO split in the other method later
    FHitResult HitResult;
    auto StartLocation = PlayerCameraManager->GetCameraLocation();
    auto EndLocation = StartLocation + Direction * RaycastLength;
    
    if (!GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECollisionChannel::ECC_BallTraceChannel
    ))
    {
        return nullptr;
    }

    auto HittedActor = Cast<ABall>(HitResult.Actor);
    return HittedActor;
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
        ECollisionChannel::ECC_TableTraceChannel
    ))
    {
        return false;
    }
    
    if (!HitResult.ImpactNormal.Equals(FVector::UpVector, 0.1))
    {
        return false;
    }
    RaycastHit = HitResult.Location;
    return true;
}

UPocketArea* ABilliardistController::TryRaycastPocketArea()
{
    int32 ViewportSizeX, ViewportSizeY;
    GetViewportSize(ViewportSizeX, ViewportSizeY);
    auto ScreenLocation = FVector2D(ViewportSizeX * CrosshairXLocation, ViewportSizeY * CrosshairYLocation);

    FVector Direction; // look direction
    if (!GetLookDirection(ScreenLocation, Direction)) return nullptr;

    // TODO split in the other method later
    FHitResult HitResult;
    auto StartLocation = PlayerCameraManager->GetCameraLocation();
    auto EndLocation = StartLocation + Direction * RaycastLength;

    if (!GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECollisionChannel::ECC_PocketAreaTracing
    ))
    {
        return nullptr;
    }

    auto HittedActor = Cast<UPocketArea>(HitResult.Component);

    return HittedActor;
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

void ABilliardistController::HandleMatchEnd()
{
    FInputModeUIOnly InputMode;
    SetInputMode(InputMode);
    bShowMouseCursor = true;

    OnMatchEnd();
}

int64 ABilliardistController::GetLocalTime()
{
    std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now().time_since_epoch()
        );
    return (int64)ms.count();
}

int64 ABilliardistController::GetNetworkTime()
{
    return GetLocalTime() + TimeOffsetFromServer;
}

void ABilliardistController::SetExaminingView()
{
    
}

void ABilliardistController::ReturnFromExaminingView()
{
    
}


void ABilliardistController::Client_GetServerTime_Implementation(int64 ServerTime)
{
    const auto LocalTime = GetLocalTime();

    // Calculate the server's system time at the moment we actually sent the request for it.
    int64 RoundTripTime = LocalTime - ServerTimeRequestWasPlaced;
    ServerTime -= RoundTripTime / 2;

    // Now calculate the difference between the two values
    TimeOffsetFromServer = ServerTime - ServerTimeRequestWasPlaced;

    // Now we can safely say that the following is true
    //
    // serverTime = timeServerTimeRequestWasPlaced + timeOffsetFromServer
    //
    // which is another way of saying
    //
    // NetworkTime = LocalTime + timeOffsetFromServer

    bTimeOffsetFromServerValid = true;
}

void ABilliardistController::Server_GetServerTime_Implementation()
{
    Client_GetServerTime(GetLocalTime());
}


bool ABilliardistController::Server_GetServerTime_Validate() { return true; }

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
