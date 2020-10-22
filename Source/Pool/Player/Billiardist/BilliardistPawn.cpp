// Copyright 2019 Andrei Vikarchuk.


#include "BilliardistPawn.h"

#include "Pool.h"

#include "Player/Components/BilliardistMovementComponent.h"
#include "Player/Components/BilliardistAimingComponent.h"
#include "Player/Components/BilliardistReplicationComponent.h"
#include "BilliardistController.h"

#include "GameplayLogic/PoolPlayerState.h"
#include "GameplayLogic/Interfaces/PlayerWithHandableBall.h"

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
    SetReplicatingMovement(true);

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

    PlayerInputComponent->BindAction("ZoomAdjust", IE_Pressed, this, &ABilliardistPawn::StartedZoomAdjustement);
    PlayerInputComponent->BindAction("ZoomAdjust", IE_Released, this, &ABilliardistPawn::FinishedZoomAdjustement);
    PlayerInputComponent->BindAction("Return", IE_Pressed, this, &ABilliardistPawn::ReturnPressHandle);
    PlayerInputComponent->BindAction("Action", IE_Pressed, this, &ABilliardistPawn::ActionPressHandle);
    PlayerInputComponent->BindAction("Action", IE_Released, this, &ABilliardistPawn::ActionReleaseHandle);
    //PlayerInputComponent->BindAction("TopView", IE_Pressed, this, &ABilliardistPawn::ExaminingPressHandle);
    PlayerInputComponent->BindAction("Ready", IE_Pressed, this, &ABilliardistPawn::ReadyStateToggle);
    PlayerInputComponent->BindAction("SpinAdjust", IE_Pressed, this, &ABilliardistPawn::StartedSpinAdjustment);
    PlayerInputComponent->BindAction("SpinAdjust", IE_Released, this, &ABilliardistPawn::FinishedSpinAdjustment);
}

void ABilliardistPawn::SetSpline(USplineComponent* Spline)
{
    if (!MovementComponent) return;
    MovementComponent->SetSpline(Spline);
}

#pragma region MovementAndInputRelated
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
    if (!(bAdjustingHitStrength || bAdjustingZoom || bAdjustingSpin))
        AddControllerYawInput(Value);
    if (bAdjustingSpin)
        AimingComponent->SpinHorizontalUpdate(Value);
}

void ABilliardistPawn::LookUp(float Value)
{
    if (bAdjustingHitStrength)
        AimingComponent->UpdateHitStrengthRatio(Value);
    else if (bAdjustingZoom)
        AimingComponent->AdjustZoom(Value);
    else if (bAdjustingSpin)
        AimingComponent->SpinVerticalUpdate(Value);
    else
        AddControllerPitchInput(Value);
}

void ABilliardistPawn::StartedZoomAdjustement()
{
    if (State == FBilliardistState::AIMING)
        bAdjustingZoom = true;
}

void ABilliardistPawn::FinishedZoomAdjustement()
{
    bAdjustingZoom = false;
}

void ABilliardistPawn::StartedSpinAdjustment()
{
    bAdjustingSpin = true;   
}

void ABilliardistPawn::FinishedSpinAdjustment()
{
    bAdjustingSpin = false;
}

#pragma endregion

void ABilliardistPawn::ActionPressHandle()
{
    switch (State)
    {
        // for less typing
        using FState = FBilliardistState;

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
    const auto TurnPlayerState = Cast<ITurnBasedPlayer>(GetPlayerState());
    if (!TurnPlayerState)
        return;

    switch (State)
    {
        // for less typing
        using FState = FBilliardistState;

        case FState::OBSERVING:
        {
            const bool bMyTurn = TurnPlayerState->GetIsMyTurn();
            if (bMyTurn)
                SetState(FBilliardistState::WALKING);
            break;
        }
        case FState::AIMING:
        {
            if (!SelectedBall)
            {
                UE_LOG(LogTemp, Warning, TEXT("Pawn: no selected ball to hit"));
                break;
            }

            bAdjustingHitStrength = false;

            float HitStrength = AimingComponent->GetHitStrength();
            // not accepting too weak hits, assuming it is not intentional
            if (HitStrength <= AimingComponent->GetMaxHitStrength() * 0.01)
                break;

            FVector LookDirection = GetControlRotation().Vector().GetSafeNormal(KINDA_SMALL_NUMBER);
            LookDirection.Z = 0; // TODO handle jump/curve later
            LaunchBall(SelectedBall, LookDirection * HitStrength);
            HandleFinishedAiming(SelectedBall);
            SelectedBall = nullptr;
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

        case FState::AIMING:
        {
            HandleFinishedAiming(SelectedBall);
            break;
        }
    }
}

void ABilliardistPawn::HandleFinishedAiming(AActor* const ActorToLookAt)
{
    AimingComponent->HandleFinishedAiming(ActorToLookAt);
    SetState(FBilliardistState::WALKING);
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

void ABilliardistPawn::OnTurnUpdate(bool NewTurn)
{
    if (NewTurn)
        SetState(FBilliardistState::WALKING);
}

// Internal player state init for client
void ABilliardistPawn::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();
}
// Internal player state init for server
void ABilliardistPawn::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
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
    DOREPLIFETIME(ABilliardistPawn, SelectedBall);
}