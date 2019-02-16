// Fill out your copyright notice in the Description page of Project Settings.

#include "BilliardistController.h"
#include "Pool.h"
#include "UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "CameraManager.h"

ABilliardistController::ABilliardistController()
{

}

bool ABilliardistController::Server_SubscribeToStateChange_Validate() { return true; }
void ABilliardistController::Server_SubscribeToStateChange_Implementation()
{
    auto Billiardist = Cast<ABilliardist>(GetPawn());
    if (Billiardist)
    {
        Billiardist->OnStateChange.AddDynamic(this, &ABilliardistController::OnPlayerStateChanged);
    }

}

void ABilliardistController::BeginPlay()
{
    Super::BeginPlay();

    Server_SubscribeToStateChange();
}

void ABilliardistController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (m_pControlledBilliardist)
    {
        auto CurrentState = m_pControlledBilliardist->GetState();

        switch (CurrentState)
        {
            case FBilliardistState::WALKING:
            {
                if (m_pPlayerSpline)
                {
                    Direction = m_pControlledBilliardist->m_fCurrentMoveDirection;

                    if (Direction != FVector::ZeroVector)
                    {
                        auto SplineTangent = m_pPlayerSpline->GetDirectionAtDistanceAlongSpline(m_fDistanceAlongSpline, ESplineCoordinateSpace::World);
                        float cosin = cosin = FVector::DotProduct(SplineTangent, Direction) /
                            (SplineTangent.Size() * Direction.Size()); // cos between spline tangent and move direction without spline
                        m_fDistanceAlongSpline += cosin * DeltaTime * m_pControlledBilliardist->GetMoveSpeed();

                        if (m_fDistanceAlongSpline >= m_pPlayerSpline->GetSplineLength())
                            m_fDistanceAlongSpline -= m_pPlayerSpline->GetSplineLength();
                        else if (m_fDistanceAlongSpline < 0)
                            m_fDistanceAlongSpline += m_pPlayerSpline->GetSplineLength();

                        Server_MovePlayer(m_pPlayerSpline->GetLocationAtDistanceAlongSpline(m_fDistanceAlongSpline,
                            ESplineCoordinateSpace::World));
                    }
                    m_pControlledBilliardist->m_fCurrentMoveDirection = FVector::ZeroVector;
                }
                break;
            }
            case FBilliardistState::PICKING:
            {
                // allow only camera controls
                // small crosshair for ball selecting is visible
                // on LBM we pick a ball and goto aiming state
                break;
            }
            case FBilliardistState::AIMING:
            {
                // we follow only the selected ball - camera flies around it
                // LBM down -> we start gaining/losing the strength meter
                // LBM up -> hit occurs, we go to the observing state
                break;
            }
            case FBilliardistState::OBSERVING:
            {
                // we are allowed to switch between different cameras aroung the whole room
                // we are not allowed to move or freely control the camera generally
                // if the player selects the camera in his own playable charater, then he can move, but he only observes
                break;
            }
            case FBilliardistState::EXAMINING:
            {
                break;
            }
        }
    }
}

void ABilliardistController::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // Replicate to everyone
    DOREPLIFETIME(ABilliardistController, m_pPlayerSpline);
    DOREPLIFETIME(ABilliardistController, m_pSelectedBall); 
    DOREPLIFETIME(ABilliardistController, m_pControlledBilliardist);
    DOREPLIFETIME(ABilliardistController, m_pCameraManager);
}

void ABilliardistController::Initialize(ATable* Table, ABilliardist* BillPawn, ACameraManager* CamMan)
{
    Server_Initialize(Table, BillPawn, CamMan);
}

bool ABilliardistController::Server_Initialize_Validate(ATable*, ABilliardist*, ACameraManager*) { return true; }
void ABilliardistController::Server_Initialize_Implementation(ATable* Table, ABilliardist* BillPawn, ACameraManager* CamMan)
{
    SetBilliardist(BillPawn);
    SetTable(Table);
    SetCameraManager(CamMan);
}

void ABilliardistController::SetBilliardist(ABilliardist* BillPawn)
{
    Server_SetBilliardist(BillPawn);
}

bool ABilliardistController::Server_SetBilliardist_Validate(ABilliardist*) { return true; }
void ABilliardistController::Server_SetBilliardist_Implementation(ABilliardist* BillPawn)
{
    if (BillPawn)
    {
        m_pControlledBilliardist = BillPawn;
    }
    else
    {
        UE_LOG(LogPool, Warning, TEXT("%s was assigned with null billiardist in Server_SetBill_Impl. Setting controlled pawn manually..."), *GetName());
        auto PossessedPawn = Cast<ABilliardist>(GetPawn());
        if (PossessedPawn)
        {
            m_pControlledBilliardist = PossessedPawn;
            UE_LOG(LogPool, Log, TEXT("%s successfully self-assigned %s as controlled Billiardist."), *GetName(), *m_pControlledBilliardist->GetName());
        }
        else
        {
            UE_LOG(LogPool, Error, TEXT("Init failed: %s could not cast its possessed pawn to the billiardist in SelfInitializePawn."), *GetName());
        }
    }
}

void ABilliardistController::SetTable(ATable* NewTable)
{
    Server_SetTable(NewTable);
}

bool ABilliardistController::Server_SetTable_Validate(ATable*) { return true; }
void ABilliardistController::Server_SetTable_Implementation(ATable* NewTable)
{
    if (NewTable)
    {
        m_pPlayerSpline = NewTable->GetSplinePath();
    }
    else
    {
        UE_LOG(LogPool, Warning, TEXT("%s was assigned with nullptr Table in Server_SetTable_Impl."), *GetName());
    }

}

void ABilliardistController::SetCameraManager(ACameraManager* CamMan)
{
    Server_SetCameraManager(CamMan);
}

bool ABilliardistController::Server_SetCameraManager_Validate(ACameraManager*) { return true; }
void ABilliardistController::Server_SetCameraManager_Implementation(ACameraManager* CamMan)
{
    if (CamMan)
    {
        m_pCameraManager = CamMan;
    }
    else
    {
        UE_LOG(LogPool, Error, TEXT("%s was assigned with nullptr CameraManager in Server_SetCamMan_Impl. Finfing camera manager manually..."), *GetName());
        for (TObjectIterator<ACameraManager> it; it; ++it)
        {
            auto FoundCamMan = *it;
            if (FoundCamMan)
            {
                m_pCameraManager = FoundCamMan;
                UE_LOG(LogPool, Log, TEXT("%s found camera manager %s and has set it manually."), *GetName(), *it->GetName());
                break;
            }
        }
        if (m_pCameraManager == nullptr)
        {
            UE_LOG(LogPool, Warning, TEXT("%s could not find any camera manager in the world. Check if one exists in the game world."), *GetName());
        }
    }
}

bool ABilliardistController::Server_MovePlayer_Validate(FVector) { return true; }

void ABilliardistController::Server_MovePlayer_Implementation(FVector NewLocation)
{
    Multicast_MovePlayer(NewLocation);
}

bool ABilliardistController::Multicast_MovePlayer_Validate(FVector NewLocation) { return true; }

void ABilliardistController::Multicast_MovePlayer_Implementation(FVector NewLocation)
{
    GetPawn()->SetActorLocation(NewLocation);
}

void ABilliardistController::SetBall(ABall* NewBall)
{
    Server_SetBall(NewBall);
}

bool ABilliardistController::Server_SetBall_Validate(ABall*) { return true; }

void ABilliardistController::Server_SetBall_Implementation(ABall* NewBall)
{
    m_pSelectedBall = NewBall;
}

// need to be run on the client as this function handles camera
void ABilliardistController::OnPlayerStateChanged(FBilliardistState NewState)
{
    Client_OnPlayerStateChanged(NewState);
}
void ABilliardistController::Client_OnPlayerStateChanged_Implementation(FBilliardistState NewState)
{
    if (m_pControlledBilliardist)
    {
        UE_LOG(LogPool, Log, TEXT("%s : controller says that its pawn's (%s) state has changed to %d"), *GetName(),
            *m_pControlledBilliardist->GetName(),
            static_cast<uint8>(m_pControlledBilliardist->GetState()));
    }
    else
    {
        UE_LOG(LogPool, Log, TEXT("Controller %s tried to log smth, but it either does not have a billiardist pawn under it."),
            *GetName());
    }

    switch (NewState)
    {
        case FBilliardistState::WALKING:
        {
            SetViewTargetWithBlend(
                GetPawn(),
                m_fCameraBlendTime
            );
            
            UE_LOG(LogPool, Warning, TEXT("%s just entered WALKING state."), *GetName());
            break;
        }
        case FBilliardistState::PICKING:
        {
            SetViewTargetWithBlend(
                GetPawn(),
                m_fCameraBlendTime
            );
            UE_LOG(LogPool, Warning, TEXT("%s just entered PICKING state."), *GetName());
            break;
        }
        case FBilliardistState::AIMING:
        {
            // blend the camera to the ball
            // add aiming widget with strengh bar and stuff
            // camera now flies around the selected ball
            UE_LOG(LogPool, Warning, TEXT("%s just entered AIMING state."), *GetName());
            break;
        }
        case FBilliardistState::OBSERVING:
        {
            UE_LOG(LogPool, Warning, TEXT("%s just entered OBSERVING state."), *GetName());
            break;
        }
        case FBilliardistState::EXAMINING:
        {
            // blend the camera to the top view
            if (!m_pCameraManager)
            {
                UE_LOG(LogPool, Error, TEXT("%s does not have camera manager assigned."));
                return;
            }
            auto Cam = m_pCameraManager->m_pTopDownCamera;
            if (Cam.Camera)
            {
                SetViewTargetWithBlend(
                    Cam.Camera,
                    Cam.fBlendTime,
                    EViewTargetBlendFunction::VTBlend_Linear,
                    0.0f,
                    Cam.bLockOutgoing
                );
            }
            else
            {
                UE_LOG(LogPool, Error, TEXT("%s tried to blend the view to top down camera, but  %s does not contain top down camera."),
                    *GetName(),
                    *m_pCameraManager->GetName());
            }   
            
            UE_LOG(LogPool, Warning, TEXT("%s just entered EXAMINING state."), *GetName());
            break;
        }
        
    }
    OnPlayerStateChangedEvent(NewState);
}

bool ABilliardistController::TryRaycastBall(ABall*& FoundBall)
{
    int32 ViewportSizeX, ViewportSizeY;
    GetViewportSize(ViewportSizeX, ViewportSizeY);
    auto ScreenLocation = FVector2D(ViewportSizeX * m_fCrosshairXLocation, ViewportSizeY * m_fCrosshairYLocation);

    FVector Direction; // look direction
    if (!GetLookDirection(ScreenLocation, Direction))
    {
        UE_LOG(LogPool, Error, TEXT("%s could not get LookDirection."), *GetName());
        return false;
    }

    // TODO split in the other method later
    FHitResult HitResult;
    auto StartLocation = PlayerCameraManager->GetCameraLocation();
    auto EndLocation = StartLocation + Direction * m_fRaycastLength;
    
    if (!GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECollisionChannel::ECC_GameTraceChannel1
    ))
    {
        UE_LOG(LogPool, Warning, TEXT("%s could not RayCast in LineTraceSingleByChannel."), *GetName());
        return false;
    }

    auto HittedActor = Cast<ABall>(HitResult.Actor);
    if (!HittedActor)
    {
        UE_LOG(LogPool, Error, TEXT("%s could not cast %s to ABall."), *GetName(), *HitResult.Actor->GetName());
        return false;
    }
    
    FoundBall = HittedActor;
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

