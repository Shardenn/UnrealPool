// Fill out your copyright notice in the Description page of Project Settings.

#include "Billiardist.h"
#include "Ball.h"
#include "Pool.h"
#include "BilliardistController.h"
#include "AimingComponent.h"

#include "Components/InputComponent.h"
#include "Components/ActorComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "UObject/UObjectIterator.h"

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

// Sets default values
ABilliardist::ABilliardist()
{
    // Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ABilliardist::BeginPlay()
{
    Super::BeginPlay();
}

void ABilliardist::Initialize(USplineComponent* NewSpline)
{
    SetSplinePath(NewSpline);
}

void ABilliardist::SetSplinePath(USplineComponent* NewSpline)
{
    SplinePath = NewSpline;
}

void ABilliardist::Tick(float DeltaTime)
{
    switch (BilliardistState)
    {
        case FBilliardistState::WALKING:
        case FBilliardistState::OBSERVING:
        {
            if (!SplinePath) 
            { 
                UE_LOG(LogPool, Warning, TEXT("%s does not have any spline path assigned."), *GetName());
                break;
            }

            if (CurrentMoveDirection != FVector::ZeroVector)
            {
                auto SplineTangent = SplinePath->GetDirectionAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World);
                float cosin = FVector::DotProduct(SplineTangent, CurrentMoveDirection) /
                    (SplineTangent.Size() * CurrentMoveDirection.Size()); // cos between spline tangent and move direction without spline
                DistanceAlongSpline += cosin * DeltaTime * GetMoveSpeed();

                if (DistanceAlongSpline >= SplinePath->GetSplineLength())
                    DistanceAlongSpline -= SplinePath->GetSplineLength();
                else if (DistanceAlongSpline < 0)
                    DistanceAlongSpline += SplinePath->GetSplineLength();

                Server_MovePlayer(SplinePath->GetLocationAtDistanceAlongSpline(DistanceAlongSpline,
                    ESplineCoordinateSpace::World));

                CurrentMoveDirection = FVector::ZeroVector;
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
            HitStrengthAlpha = (CurrentHitStrength - HitStrengthMin) /
                (HitStrengthMax - HitStrengthMin);

            float koeff = FMath::Lerp(HitStrengthChangeSpeed, HitStrengthChangeHigh,
                HitStrengthAlpha);

            HitStrengthAlpha += DeltaTime * koeff * (StrengthIncreasing ? 1 : -1);

            if (HitStrengthAlpha > 1.f)
            {
                HitStrengthAlpha = 1.f;
                StrengthIncreasing = false;
            }
            if (HitStrengthAlpha < 0.f)
            {
                HitStrengthAlpha = 0.f;
                StrengthIncreasing = true;
            }

            CurrentHitStrength = HitStrengthMin + HitStrengthAlpha *
                (HitStrengthMax - HitStrengthMin);

            break;
        }

        case FBilliardistState::EXAMINING:
        {
            break;
        }
    }
}

void ABilliardist::SetupPlayerInputComponent(UInputComponent* InputComponent)
{
    Super::SetupPlayerInputComponent(InputComponent);

    InputComponent->BindAxis("MoveForward", this, &ABilliardist::MoveForward);
    InputComponent->BindAxis("MoveRight", this, &ABilliardist::MoveRight);
    //InputComponent->BindAxis("Turn", this, &ABilliardist::Turn);
    //InputComponent->BindAxis("LookUp", this, &ABilliardist::LookUp);

    InputComponent->BindAction("Action", IE_Pressed, this, &ABilliardist::ActionPressHandle);
    InputComponent->BindAction("Return", IE_Pressed, this, &ABilliardist::ReturnPressHandle);
    InputComponent->BindAction("TopView", IE_Pressed, this, &ABilliardist::ExaminingPressHandle);
}

void ABilliardist::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // Replicate to everyone
    DOREPLIFETIME(ABilliardist, SplinePath);
    DOREPLIFETIME(ABilliardist, SelectedBall);
    DOREPLIFETIME(ABilliardist, BilliardistState);
    DOREPLIFETIME(ABilliardist, PreviousState);
}

void ABilliardist::MoveForward(float Value)
{
    if (Value == 0.0f)
        return;

    auto Rotation = GetControlRotation();
    const auto Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::X);
    CurrentMoveDirection += Direction * Value;
}

void ABilliardist::MoveRight(float Value)
{
    if (Value == 0.0f)
        return;

    auto Rotation = GetControlRotation();
    const auto Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::Y);
    CurrentMoveDirection += Direction * Value;
}

bool ABilliardist::Server_MovePlayer_Validate(FVector) { return true; }
void ABilliardist::Server_MovePlayer_Implementation(FVector NewLocation)
{
    Multicast_MovePlayer(NewLocation);
}

void ABilliardist::Multicast_MovePlayer_Implementation(FVector NewLocation)
{
    SetActorLocation(NewLocation);
}

void ABilliardist::Turn(float Value)
{
    AddControllerYawInput(Value * MouseSenseX);
}

void ABilliardist::LookUp(float Value)
{
    AddControllerPitchInput(Value * MouseSenseY);
}

void ABilliardist::ActionPressHandle()
{
    switch (BilliardistState)
    {
        case FBilliardistState::WALKING:
        {
            SetState(FBilliardistState::PICKING);
            break;
        }
        case FBilliardistState::PICKING:
        {
            // when we press LMB while PIKING and we found some ball, we should 
            // 1. set the selected ball
            ABall* FoundBall = nullptr;
            ABilliardistController* Controller = Cast<ABilliardistController>(GetController());

            if (!ensure(Controller)) { return; }

            if (Controller->TryRaycastBall(FoundBall))
            {
                UE_LOG(LogPool, Log, TEXT("Found ball %s "), *GetName());
                SetSelectedBall(FoundBall);
                SetState(FBilliardistState::AIMING);
                UE_LOG(LogPool, Warning, TEXT("%s just entered AIMING state."), *GetName());

                // 2. switch the pawn to aiming camera
                // TODO camera handling here
            }

            break;
        }
        case FBilliardistState::AIMING:
        {
            // TODO where to hangle it? BP or ++ ?
            SetState(FBilliardistState::OBSERVING);
            break;

            if (!ensure(SelectedBall)) { return; }

            // get the current hit strength and look vector
            auto AimingComponent = GetComponentByClass(TSubclassOf<UAimingComponent>());
            if (!ensure(AimingComponent)) { return; }
            auto BallHitDirection = Cast<USceneComponent>(AimingComponent)->GetComponentToWorld().Rotator().Vector();

            BallHitDirection.Z = 0.f;
            auto hitVector = BallHitDirection * CurrentHitStrength;

            // nil hit strength related stuff
            HitStrengthAlpha = 0.f;
            CurrentHitStrength = HitStrengthMin;

            // TODO ball launch here and camera handling
            LaunchBall(SelectedBall, hitVector);
            SetSelectedBall(nullptr);

            

            break;
        }
        case FBilliardistState::OBSERVING:
        {
            // set any state, but it is possible only to set examining (handled in setstate)
            // TODO camera switching here

            break;
        }
        case FBilliardistState::EXAMINING:
        {
            break;
        }
    }
}

void ABilliardist::ReturnPressHandle()
{
    switch (BilliardistState)
    {
        case FBilliardistState::WALKING:
        {
            // nowhere to return, it is a default state
            break;
        }
        case FBilliardistState::PICKING:
        {
            SetState(FBilliardistState::WALKING);
            SetSelectedBall(nullptr);
            UE_LOG(LogPool, Warning, TEXT("%s just entered WALKING state."), *GetName());
            break;
        }
        case FBilliardistState::AIMING:
        {
            // TODO returning from aiming
            SetState(FBilliardistState::PICKING);

            HitStrengthAlpha = 0.f;
            CurrentHitStrength = HitStrengthMin;
            break;
        }
        case FBilliardistState::OBSERVING:
        {
            // set examining
            SetState(FBilliardistState::WALKING);
            // we cannot return to anything except examining
            break;
        }
        case FBilliardistState::EXAMINING:
        {
            // return to the previous state
            SetState(PreviousState);
            UE_LOG(LogPool, Warning, TEXT("%s just entered previous state from EXAMINING state."), *GetName());
            break;
        }
    }
}

void ABilliardist::ExaminingPressHandle()
{
    // TODO split in two funcs - GoToExamining and ReturnFromExaminging
    if (BilliardistState != FBilliardistState::EXAMINING)
    {
        SetState(FBilliardistState::EXAMINING);
    }
    else
    {
        SetState(PreviousState);
    }
}

void ABilliardist::SetState(FBilliardistState NewState)
{
    Server_SetState(NewState);
}

bool ABilliardist::Server_SetState_Validate(FBilliardistState) { return true; }

void ABilliardist::Server_SetState_Implementation(FBilliardistState NewState)
{
    if (BilliardistState == NewState)
        return;

    // TODO remove "true ||" here. Added for debugging OBSERVING state
    if (true || BillStateMachine[(int)BilliardistState][(int)NewState] == 1) // only if state machine allows us the queried state transfer
                                                  // then we update the state. It is replicated automatically
                                                  // by UPROPERTY
    {
        PreviousState = BilliardistState;
        BilliardistState = NewState;

        OnStateChange.Broadcast(BilliardistState, PreviousState);
        OnPlayerStateChanged(BilliardistState, PreviousState);
    }
}

void ABilliardist::SetSelectedBall(ABall* NewBall)
{
    Server_SetSelectedBall(NewBall);
}

bool ABilliardist::Server_SetSelectedBall_Validate(ABall*) { return true; }
void ABilliardist::Server_SetSelectedBall_Implementation(ABall* NewBall)
{
    SelectedBall = NewBall;
    OnSelectedBallUpdate.Broadcast(SelectedBall);
}


void ABilliardist::LaunchBall(ABall* Ball, FVector Velocity)
{
    Server_LaunchBall(Ball, Velocity);
}

bool ABilliardist::Server_LaunchBall_Validate(ABall*, FVector) { return true; }
void ABilliardist::Server_LaunchBall_Implementation(ABall* Ball, FVector Velocity)
{
    Multicast_LaunchBall(Ball, Velocity);
}

void ABilliardist::Multicast_LaunchBall_Implementation(ABall* Ball, FVector Velocity)
{
    Cast<UStaticMeshComponent>(Ball->GetRootComponent())->AddForce(Velocity);
}