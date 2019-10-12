// Copyright 2019 Andrei Vikarchuk.


#include "BilliardistPawn.h"

#include "Pool.h"
#include "BilliardistMovementComponent.h"
#include "BilliardistAimingComponent.h"
#include "BilliardistReplicationComponent.h"
#include "BilliardistController.h"
#include "Objects/Ball.h"

#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"

ABilliardistPawn::ABilliardistPawn()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
    bReplicateMovement = true;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    SetRootComponent(Mesh);

    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring arm"));
    SpringArm->bUsePawnControlRotation = true;
    SpringArm->TargetArmLength = 60.f; // sensible default
    SpringArm->SetupAttachment(RootComponent);

    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SpringArm);

    MovementComponent = CreateDefaultSubobject<UBilliardistMovementComponent>(TEXT("Movement component"));
    AimingComponent = CreateDefaultSubobject<UBilliardistAimingComponent>(TEXT("Aiming component"));
    ReplicationComponent = CreateDefaultSubobject<UBilliardistReplicationComponent>(TEXT("Replication component"));
}

void ABilliardistPawn::BeginPlay()
{
    Super::BeginPlay();

    if (ReplicationComponent)
    {
        auto PlayerState = GetPlayerState();
        ReplicationComponent->SetPlayerState(PlayerState);
    }
}

void ABilliardistPawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ABilliardistPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAxis("MoveRight", this, &ABilliardistPawn::MoveRight);
    PlayerInputComponent->BindAxis("MoveForward", this, &ABilliardistPawn::MoveForward);
    PlayerInputComponent->BindAxis("Turn", this, &ABilliardistPawn::Turn);
    PlayerInputComponent->BindAxis("LookUp", this, &ABilliardistPawn::LookUp);

    PlayerInputComponent->BindAction("Return", IE_Pressed, this, &ABilliardistPawn::ReturnPressHandle);
    PlayerInputComponent->BindAction("Action", IE_Pressed, this, &ABilliardistPawn::ActionPressHandle);
    //PlayerInputComponent->BindAction("TopView", IE_Pressed, this, &ABilliardistPawn::ExaminingPressHandle);
    PlayerInputComponent->BindAction("Ready", IE_Pressed, this, &ABilliardistPawn::ReadyStateToggle);
}

void ABilliardistPawn::SetSpline(USplineComponent* Spline)
{
    if (!MovementComponent) return;
    MovementComponent->SetSpline(Spline);
}

void ABilliardistPawn::MoveForward(float Value)
{
    if (!MovementComponent) return;

    if (State == FBilliardistState::WALKING ||
        State == FBilliardistState::PICKING)
        MovementComponent->SetForwardIntent(Value);
}

void ABilliardistPawn::MoveRight(float Value)
{
    if (!MovementComponent) return;

    if (State == FBilliardistState::WALKING ||
        State == FBilliardistState::PICKING)
        MovementComponent->SetRightIntent(Value);
}

void ABilliardistPawn::Turn(float Value)
{
    AddControllerYawInput(Value);
}

void ABilliardistPawn::LookUp(float Value)
{
    AddControllerPitchInput(Value);
}

void ABilliardistPawn::ActionPressHandle()
{
    switch (State)
    {
        // for less typing
        using FState = FBilliardistState;

        case FState::WALKING:
        {
            SetState(FState::PICKING);
            break;
        }
        case FState::PICKING:
        {
            auto BillController = Cast<ABilliardistController>(Controller);
            if (!BillController) { return; }

            ABall* Ball = nullptr;
            BillController->TryRaycastBall(Ball);
            if (Ball)
            {
                SelectedBall = Ball;
                HandleBallSelected(Ball);
            }
            break;
        }
        case FState::AIMING:
        {
            FVector LookDirection = GetControlRotation().Vector();
            LookDirection.Z = 0; // TODO handle jump/curve later
            auto BallRoot = Cast<UPrimitiveComponent>(SelectedBall->GetRootComponent());
            if (BallRoot) 
                BallRoot->AddImpulse(LookDirection * AimingComponent->GetHitStrength());
            SetState(FState::OBSERVING);
            SelectedBall = nullptr;
            HandleFinishedAiming();
            break;
        }
    }
}

void ABilliardistPawn::HandleBallSelected(ABall* Ball)
{
    AimingComponent->HandleStartedAiming(Ball->GetActorLocation());
    SetState(FBilliardistState::AIMING);
}

void ABilliardistPawn::ReturnPressHandle()
{
    switch (State)
    {
        // for less typing
        using FState = FBilliardistState;

        case FState::PICKING:
        {
            SetState(FState::WALKING);
            break;
        }
        case FState::AIMING:
        {
            HandleFinishedAiming();
            break;
        }
    }
}

void ABilliardistPawn::HandleFinishedAiming()
{
    AimingComponent->HandleFinishedAiming();
    SetState(FBilliardistState::PICKING);
}

void ABilliardistPawn::ReadyStateToggle()
{
    if (!ReplicationComponent) return;
    ReplicationComponent->ReadyStateToggle();
}


void ABilliardistPawn::SetState(const FBilliardistState& NewState)
{
    State = NewState;
    OnStateChange.Broadcast(State);
}
