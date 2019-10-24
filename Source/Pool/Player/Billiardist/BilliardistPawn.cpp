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

#include "UnrealNetwork.h" // still needed for State replication, though we have RepComp

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

    ABilliardistController* BillController = Cast<ABilliardistController>(GetController());
    if (BillController)
        BillController->SubscribeToPlayerStateChange(this);
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
    APoolPlayerState* BillPlayerState = Cast<APoolPlayerState>(GetPlayerState());
    // If not our turn - terminate
    if (!ensure(BillPlayerState != nullptr)) return;

    if (!BillPlayerState->GetIsMyTurn())
        return;

    // If we have a ball in hand - try place it
    if (BillPlayerState->GetIsBallInHand())
    {
        TryPlaceCueBall(BillPlayerState);
        return;
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

void ABilliardistPawn::TryPlaceCueBall(APoolPlayerState* InPlayerState)
{
    auto BillController = Cast<ABilliardistController>(GetController());
    if (!ensure(BillController != nullptr)) return;

    FVector TableHitResult;
    if (BillController->TryRaycastTable(TableHitResult))
    {
        InPlayerState->PlaceCueBall(TableHitResult);
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
    ReplicationComponent->Server_PerformBallHit(Ball, Velocity);
}

void ABilliardistPawn::ReadyStateToggle()
{
    APoolPlayerState* BillPlayerState = Cast<APoolPlayerState>(GetPlayerState());
    if (BillPlayerState) BillPlayerState->Server_ToggleReady();
}

void ABilliardistPawn::NotifyTurnUpdate(bool NewTurn)
{
    UE_LOG(LogPool, Warning, TEXT("%s : now my new turn state is %d"), *GetName(), NewTurn);
    if (NewTurn)
        SetState(FBilliardistState::WALKING);
}

void ABilliardistPawn::SetState(const FBilliardistState& NewState)
{
    State = NewState;
    OnStateChange.Broadcast(State);
    Server_SetState(NewState);
}

bool ABilliardistPawn::Server_SetState_Validate(const FBilliardistState& NewState) { return true; }
void ABilliardistPawn::Server_SetState_Implementation(const FBilliardistState& NewState)
{
    State = NewState;
}

float ABilliardistPawn::GetMaxHitStrength()
{
    return AimingComponent->GetMaxHitStrength();
}

float ABilliardistPawn::GetCurrentHitStrength()
{
    return AimingComponent->GetHitStrength();
}

void ABilliardistPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ABilliardistPawn, State);
}