// Fill out your copyright notice in the Description page of Project Settings.

#include "BilliardistController.h"
#include "Billiardist.h"
#include "Pool.h"
#include "Components/InputComponent.h"
#include "Camera/CameraComponent.h"
#include "UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "CameraManager.h"
#include "AimingCamera.h"

#ifndef STATE_MACHINE
#define STATE_MACHINE
// observing is the state that takes place after a hit - when we are waiting for the balls to stop
int BillStateMachine[5][5] = { // state machine of transferring from one state to another
    // W, P, A, O, E
    { 1, 1, 1, 0, 1 }, // Walking
    { 1, 1, 1, 0, 1 }, // Picking
    { 0, 1, 1, 1, 1 }, // Aiming - cant return directly to moving
    { 0, 0, 0, 1, 1 }, // Observing - cant return to any state
    { 1, 1, 1, 1, 1 }  // Examining
};
#endif

ABilliardistController::ABilliardistController()
{

}


void ABilliardistController::BeginPlay()
{
    Super::BeginPlay();
}

void ABilliardistController::SetupInputComponent()
{
    Super::SetupInputComponent();

    InputComponent->BindAxis("MoveForward", this, &ABilliardistController::MoveForward);
    InputComponent->BindAxis("MoveRight", this, &ABilliardistController::MoveRight);
    InputComponent->BindAxis("Turn", this, &ABilliardistController::AddYawInput);
    InputComponent->BindAxis("LookUp", this, &ABilliardistController::AddPitchInput);

    InputComponent->BindAction("Action", IE_Pressed, this, &ABilliardistController::ActionPressHandle);
    InputComponent->BindAction("Return", IE_Pressed, this, &ABilliardistController::ReturnPressHandle);
    InputComponent->BindAction("TopView", IE_Pressed, this, &ABilliardistController::ExaminingPressHandle);
}

void ABilliardistController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (m_pControlledBilliardist)
    {
        switch (m_eState)
        {
            case FBilliardistState::WALKING:
            {
                if (!m_pPlayerSpline)
                {
                    UE_LOG(LogPool, Error, TEXT("%s does not have a m_pPlayerSpline assigned"), *GetName());
                    break;
                }

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
    DOREPLIFETIME(ABilliardistController, m_eState);
    DOREPLIFETIME(ABilliardistController, m_ePreviousState);
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
        m_pPlayerSpline = NewTable->GetSplinePath();
    else
        UE_LOG(LogPool, Warning, TEXT("%s was assigned with nullptr Table in Server_SetTable_Impl."), *GetName());
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
    OnSelectedBallUpdate.Broadcast(NewBall);
}

bool ABilliardistController::Server_SetBall_Validate(ABall*) { return true; }

void ABilliardistController::Server_SetBall_Implementation(ABall* NewBall)
{
    m_pSelectedBall = NewBall;
}

bool ABilliardistController::Server_SwitchPawn_Validate(APawn*) { return true; }

void ABilliardistController::Server_SwitchPawn_Implementation(APawn* newPawn)
{
    UnPossess();
    Possess(newPawn);
    newPawn->SetOwner(this);
}

void ABilliardistController::SetState(FBilliardistState NewState)
{
    Server_SetState(NewState);
}

bool ABilliardistController::Server_SetState_Validate(FBilliardistState) { return true; }

void ABilliardistController::Server_SetState_Implementation(FBilliardistState NewState)
{
    if (m_eState == NewState)
        return;

    if (BillStateMachine[(int)m_eState][(int)NewState] == 1) // only if state machine allows us the queried state transfer
                                                  // then we update the state. It is replicated automatically
                                                  // by UPROPERTY
    {
        m_ePreviousState = m_eState;
        m_eState = NewState;
        OnStateChange.Broadcast(m_eState);
        OnPlayerStateChangedEvent(m_eState);
    }
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

void ABilliardistController::SwitchPawn(APawn* newPawn)
{
    Server_SwitchPawn(newPawn);
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

void ABilliardistController::MoveForward(float Value)
{
    if (Value == 0.0f || Cast<ABilliardist>(GetPawn()) == nullptr)
        return;

    auto Rotation = GetControlRotation();

    const auto Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::X);
    
    auto Bill = Cast<ABilliardist>(GetPawn());
    if (!Bill)
    {
        UE_LOG(LogPool, Error, TEXT("%s could not cast its pawn to ABilliardist in MoveForward"),
            *GetName());
        return;
    }
    
    Bill->m_fCurrentMoveDirection += Direction * Value;
}

void ABilliardistController::MoveRight(float Value)
{
    if (Value == 0.0f || Cast<ABilliardist>(GetPawn()) == nullptr)
        return;

    auto Rotation = GetControlRotation();

    const auto Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::Y);
    
    auto Bill = Cast<ABilliardist>(GetPawn());
    if (!Bill)
    {
        UE_LOG(LogPool, Error, TEXT("%s could not cast its pawn to ABilliardist in MoveRight"),
            *GetName());
        return;
    }

    Bill->m_fCurrentMoveDirection += Direction * Value;
}

void ABilliardistController::ActionPressHandle()
{
    switch (m_eState)
    {
        case FBilliardistState::WALKING:
        {
            SetState(FBilliardistState::PICKING);

            UE_LOG(LogPool, Warning, TEXT("%s just entered WALKING state."), *GetName());
            break;
        }
        case FBilliardistState::PICKING:
        {
            // when we press LMB while PIKING and we found some ball, we should 
            // 1. set the selected ball
            ABall* FoundBall = nullptr;
            if (TryRaycastBall(FoundBall))
            {
                UE_LOG(LogPool, Log, TEXT("Found ball %s "), *GetName());
                SetBall(FoundBall);
                SetState(FBilliardistState::AIMING);
                UE_LOG(LogPool, Warning, TEXT("%s just entered AIMING state."), *GetName());

                // 2. switch the pawn to aiming camera
                APawn* aimCamera = nullptr;
                if (m_pCameraManager && m_pCameraManager->AimingPawn)
                    aimCamera = m_pCameraManager->AimingPawn;

                auto location = PlayerCameraManager->GetCameraLocation();
                auto rotation = PlayerCameraManager->GetCameraRotation();

                aimCamera->SetActorLocationAndRotation(location, rotation);

                Server_SwitchPawn(aimCamera);
            }
            
            break;
        }
        case FBilliardistState::AIMING:
        {
            // set observing
            // 1. get the current hit strength and look vector

            // 2. handle ball push
            // 3. in this state it is possible to switch between additional cameras
            break;
        }
        case FBilliardistState::OBSERVING:
        {
            // set any state, but it is possible only to set examining (handled in setstate)
            break;
        }
        case FBilliardistState::EXAMINING:
        {
            // return to the previous state
            break;
        }
    }
}

void ABilliardistController::ReturnPressHandle()
{
    switch (m_eState)
    {
        case FBilliardistState::WALKING:
        {
            // nowhere to return, it is a default state
            break;
        }
        case FBilliardistState::PICKING:
        {
            SetState(FBilliardistState::WALKING);
            UE_LOG(LogPool, Warning, TEXT("%s just entered WALKING state."), *GetName());
            break;
        }
        case FBilliardistState::AIMING:
        {
            // when we are aiming, Billiardist is not current Pawn.
            // Therefore, it does not have active owning connection
            // and we process all input in BP_AimingCamera

            // 1. clear selected ball
            SetBall(nullptr);
            // 2. set picking
            SetState(FBilliardistState::PICKING);
            UE_LOG(LogPool, Warning, TEXT("%s just entered PICKING state."), *GetName());
            // 3. return to default pawn if we are not controlling it
            if (Cast<ABilliardist>(GetPawn()) == nullptr)
                Server_SwitchPawn(m_pControlledBilliardist);

            break;
        }
        case FBilliardistState::OBSERVING:
        {
            // set examining
            // we cannot return to anything except examining
            break;
        }
        case FBilliardistState::EXAMINING:
        {
            // return to the previous state
            SetState(m_ePreviousState);
            UE_LOG(LogPool, Warning, TEXT("%s just entered previous state from EXAMINING state."), *GetName());
            break;
        }
    }
}

void ABilliardistController::ExaminingPressHandle()
{
    if (m_eState != FBilliardistState::EXAMINING)
        SetState(FBilliardistState::EXAMINING);
    else
        SetState(m_ePreviousState);
}

