// Copyright 2019 Andrei Vikarchuk.

#include "Billiardist.h"
// TODO doubtful include. Needs refactoring
#include "Objects/Ball.h"
#include "Objects/BallAmerican.h"

#include "Pool.h"
#include "BilliardistController.h"
#include "AimingComponent.h"
#include "AmericanPool/PoolPlayerState.h"
#include "AmericanPool/PoolGameMode.h"
#include "AmericanPool/PoolGameState.h"

#include "Components/InputComponent.h"
#include "Components/ActorComponent.h"
#include "Components/SplineComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

#include "UObject/UObjectIterator.h"
#include "DrawDebugHelpers.h"


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
    bReplicates = true;
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
    // TODO getPlayerState every tick. Warning!
    APoolPlayerState* PoolPlayerState = Cast<APoolPlayerState>(GetPlayerState());

    if (PoolPlayerState && PoolPlayerState->GetIsBallInHand())
    {
        FVector TablePoint;

        ABilliardistController* BillController = Cast<ABilliardistController>(GetController());
        if (!ensure(BillController)) { return; }

        if (BillController->TryRaycastTable(TablePoint))
        {
            DrawDebugSphere(GetWorld(), TablePoint, 5, 8, FColor::Red);
            //PoolPlayerState->PlaceCueBall(TablePoint);
        }
    }

    switch (BilliardistState)
    {
        case FBilliardistState::WALKING:
        case FBilliardistState::OBSERVING:
        {
            if (!SplinePath) 
            { 
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

                FVector NewLocation = SplinePath->GetLocationAtDistanceAlongSpline(DistanceAlongSpline,
                    ESplineCoordinateSpace::World);
                MovePlayer(NewLocation);

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

void ABilliardist::SetupPlayerInputComponent(UInputComponent* InInputComponent)
{
    Super::SetupPlayerInputComponent(InInputComponent);

    InInputComponent->BindAxis("MoveForward", this, &ABilliardist::MoveForward);
    InInputComponent->BindAxis("MoveRight", this, &ABilliardist::MoveRight);
    //InputComponent->BindAxis("Turn", this, &ABilliardist::Turn);
    //InputComponent->BindAxis("LookUp", this, &ABilliardist::LookUp);

    InInputComponent->BindAction("Action", IE_Pressed, this, &ABilliardist::ActionPressHandle);
    InInputComponent->BindAction("Return", IE_Pressed, this, &ABilliardist::ReturnPressHandle);
    InInputComponent->BindAction("TopView", IE_Pressed, this, &ABilliardist::ExaminingPressHandle);
    InInputComponent->BindAction("Ready", IE_Pressed, this, &ABilliardist::ReadyPressHandle);
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

void ABilliardist::MovePlayer(FVector NewLocation)
{
    SetActorLocation(NewLocation);
    Server_MovePlayer(NewLocation);
}

bool ABilliardist::Server_MovePlayer_Validate(FVector) { return true; }
void ABilliardist::Server_MovePlayer_Implementation(FVector NewLocation)
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

void ABilliardist::ReadyPressHandle()
{
    APoolPlayerState* PoolPlayerState = Cast<APoolPlayerState>(GetPlayerState());
    if (!ensure(PoolPlayerState != nullptr)) return;
    PoolPlayerState->ToggleReady();
}

void ABilliardist::ActionPressHandle()
{
    APoolPlayerState* PoolPlayerState = Cast<APoolPlayerState>(GetPlayerState());
    
    if (PoolPlayerState && PoolPlayerState->GetIsBallInHand())
    {
        UE_LOG(LogPool, Warning, TEXT("%s has ball in hand"), *GetName());
        FVector TablePoint;

        ABilliardistController* BillController = Cast<ABilliardistController>(GetController());
        if (!ensure(BillController)) { return; }

        if (BillController->TryRaycastTable(TablePoint))
        {
            //DrawDebugSphere(GetWorld(), TablePoint, 1, 8, FColor::Red, true, -1, 0, 2);
            PoolPlayerState->PlaceCueBall(TablePoint);
        }

        return;
    }

    switch (BilliardistState)
    {
        case FBilliardistState::WALKING:
        {
            if (!Cast<APoolPlayerState>(GetPlayerState())->GetIsMyTurn())
                break;
            
            SetState(FBilliardistState::PICKING);
            break;
        }
        case FBilliardistState::PICKING:
        {
            if (!Cast<APoolPlayerState>(GetPlayerState())->GetIsMyTurn())
                break;
            // when we press LMB while PIKING and we found some ball, we should 
            // 1. set the selected ball
            ABall* FoundBall = nullptr;
            ABilliardistController* BillController = Cast<ABilliardistController>(GetController());

            if (!ensure(BillController)) { return; }

            if (BillController->TryRaycastBall(FoundBall))
            {
                ABallAmerican* AmericanBall = Cast<ABallAmerican>(FoundBall);
                if (AmericanBall && AmericanBall->GetType() == FBallType::Cue)
                {
                    SetSelectedBall(FoundBall);
                    SetState(FBilliardistState::AIMING);
                }

                // 2. switch the pawn to aiming camera
                // TODO camera handling here
            }

            break;
        }
        case FBilliardistState::AIMING:
        {
            if (!Cast<APoolPlayerState>(GetPlayerState())->GetIsMyTurn())
                break;
            // TODO where to hangle it? BP or ++ ?
            SetState(FBilliardistState::OBSERVING);
            break;

            if (!ensure(SelectedBall)) { return; }
            // Finish turn locally
            Cast<APoolPlayerState>(GetPlayerState())->SetIsMyTurn(false);
            // Turn to the next player will be handled automatically
            // on server after all balls stop moving

            // get the current hit strength and look vector
            auto AimingComponent = GetComponentByClass(TSubclassOf<UAimingComponent>());
            if (!ensure(AimingComponent)) { return; }
            auto BallHitDirection = Cast<USceneComponent>(AimingComponent)->GetComponentToWorld().Rotator().Vector();

            BallHitDirection.Z = 0.f;
            auto hitVector = BallHitDirection * CurrentHitStrength;

            // nil hit strength related stuff
            HitStrengthAlpha = 0.f;
            CurrentHitStrength = HitStrengthMin;

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
        case FBilliardistState::PICKING:
        {
            SetState(FBilliardistState::WALKING);
            SetSelectedBall(nullptr);
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

        OnPlayerStateChanged(BilliardistState, PreviousState);
    }
}

void ABilliardist::OnRep_StateReplicated()
{
    OnStateChange.Broadcast(BilliardistState, PreviousState);
}

void ABilliardist::SetSelectedBall(ABall* NewBall)
{
    Server_SetSelectedBall(NewBall);
}

bool ABilliardist::Server_SetSelectedBall_Validate(ABall*) { return true; }
void ABilliardist::Server_SetSelectedBall_Implementation(ABall* NewBall)
{
    SelectedBall = NewBall;
}

void ABilliardist::OnRep_SelectedBallReplicated()
{
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

bool ABilliardist::Multicast_LaunchBall_Validate(ABall* Ball, FVector Velocity) { return true; }
void ABilliardist::Multicast_LaunchBall_Implementation(ABall* Ball, FVector Velocity)
{
    if (HasAuthority())
    {
        UWorld* World = GetWorld();
        APoolGameState* State = Cast<APoolGameState>(UGameplayStatics::GetGameState(World));
        if (!ensure(State != nullptr)) return;

        State->StartWatchingBallsMovement();
    }

    Cast<UStaticMeshComponent>(Ball->GetRootComponent())->AddImpulse(Velocity, NAME_None, false);
}