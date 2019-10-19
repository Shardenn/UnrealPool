// Copyright 2019 Andrei Vikarchuk.


#include "BilliardistPawn.h"

#include "Pool.h"
#include "BilliardistMovementComponent.h"
#include "BilliardistAimingComponent.h"
#include "BilliardistReplicationComponent.h"
#include "BilliardistController.h"
#include "AmericanPool/PoolPlayerState.h"
#include "AmericanPool/PoolGameState.h"
#include "Objects/Ball.h"

#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

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
        auto PlState = GetPlayerState();
        ReplicationComponent->SetPlayerState(PlState);
    }

    APoolPlayerState* MyPlayerState = Cast<APoolPlayerState>(GetPlayerState());
    if (MyPlayerState)
        MyPlayerState->OnPlayerTurnChange.AddDynamic(this, &ABilliardistPawn::OnTurnStateUpdate);
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
    PlayerInputComponent->BindAction("Action", IE_Released, this, &ABilliardistPawn::ActionReleaseHandle);
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

    if (State != FBilliardistState::EXAMINING &&
        State != FBilliardistState::AIMING)
        MovementComponent->SetForwardIntent(Value);
}

void ABilliardistPawn::MoveRight(float Value)
{
    if (!MovementComponent) return;

    if (State != FBilliardistState::EXAMINING &&
        State != FBilliardistState::AIMING)
        MovementComponent->SetRightIntent(Value);
}

void ABilliardistPawn::Turn(float Value)
{
    if (!bAdjustingHitStrength)
        AddControllerYawInput(Value);
}

void ABilliardistPawn::LookUp(float Value)
{
    if (bAdjustingHitStrength)
        AimingComponent->UpdateHitStrengthRatio(Value);
    else
        AddControllerPitchInput(Value);
}

void ABilliardistPawn::ActionPressHandle()
{
    auto PlayerState = ReplicationComponent->GetPlayerState();
    // If not our turn - terminate
    if (!PlayerState->GetIsMyTurn())
        return;

    // If we have a ball in hand - try place it
    if (PlayerState->GetIsBallInHand())
    {
        auto Controller = Cast<ABilliardistController>(GetController());

        FVector TableHitResult;
        if (Controller->TryRaycastTable(TableHitResult))
        {
            PlayerState->PlaceCueBall(TableHitResult);
        }
    }

    switch (State)
    {
        // for less typing
        using FState = FBilliardistState;

        case FState::WALKING:
        {
            SetState(FState::PICKING);
            break;
        }
        case FState::AIMING:
        {
            bAdjustingHitStrength = true;
            break;
        }
    }
}

void ABilliardistPawn::HandleBallSelected(ABall* Ball)
{
    AimingComponent->HandleStartedAiming(Ball->GetActorLocation());
    SetState(FBilliardistState::AIMING);
}

void ABilliardistPawn::ActionReleaseHandle()
{
    switch (State)
    {
        // for less typing
        using FState = FBilliardistState;

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
            bAdjustingHitStrength = false;

            float HitStrength = AimingComponent->GetHitStrength();
            // not accepting too weak hits, assuming it is not intentional
            if (HitStrength <= AimingComponent->GetMaxHitStrength() * 0.01)
                break;

            FVector LookDirection = GetControlRotation().Vector();
            LookDirection.Z = 0; // TODO handle jump/curve later
            LaunchBall(SelectedBall, LookDirection * HitStrength);
            SelectedBall = nullptr;
            HandleFinishedAiming();
            SetState(FState::OBSERVING);
            break;
        }
    }
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

void ABilliardistPawn::LaunchBall(ABall* Ball, const FVector& Velocity)
{
    UWorld* World = GetWorld();
    APoolGameState* State = Cast<APoolGameState>(UGameplayStatics::GetGameState(World));
    if (!ensure(State != nullptr)) return;

    State->StartWatchingBallsMovement();

    Cast<UStaticMeshComponent>(Ball->GetRootComponent())->AddImpulse(Velocity, NAME_None, false);
}

void ABilliardistPawn::ReadyStateToggle()
{
    if (!ReplicationComponent) return;
    ReplicationComponent->ReadyStateToggle();
}

void ABilliardistPawn::OnTurnStateUpdate(bool IsMyTurn)
{
    UE_LOG(LogPool, Warning, TEXT("%s : now my new turn state is %d"), *GetName(), IsMyTurn);
    if (IsMyTurn)
        SetState(FBilliardistState::WALKING);
}


void ABilliardistPawn::SetState(const FBilliardistState& NewState)
{
    State = NewState;
    OnStateChange.Broadcast(State);
}
