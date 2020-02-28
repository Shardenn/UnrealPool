// Copyright 2019 Andrei Vikarchuk.

#include "Ball.h"
#include "Pool.h"
#include "GameplayLogic/PoolGameState.h"
#include "BilliardistController.h"

#include "Kismet/GameplayStatics.h"
#include "OnlineSubsystem.h" // LogOnlineGame

// Sets default values
ABall::ABall()
{
    SetReplicates(true);
    SetReplicateMovement(false);

    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bTickEvenWhenPaused = true;
    PrimaryActorTick.TickGroup = TG_PrePhysics;
    SetActorTickEnabled(true);

    SphereMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Sphere mesh"));
    SetRootComponent(SphereMesh);
    
    SphereMesh->SetSimulatePhysics(true);
    SphereMesh->SetEnableGravity(true);
    SphereMesh->BodyInstance.bGenerateWakeEvents = true;
    //SphereMesh->SetMassOverrideInKg(NAME_None, 0.2);
    SphereMesh->SetAngularDamping(0.6);
    SphereMesh->SetLinearDamping(0.2);

    SphereMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    SphereMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);

    ProxyStates.Init(FSmoothPhysicsState(), PROXY_STATE_ARRAY_SIZE_MAX);
}

// Called when the game starts or when spawned
void ABall::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        APoolGameState* GameState = Cast<APoolGameState>(UGameplayStatics::GetGameState(GetWorld()));
        if (!ensure(GameState != nullptr)) return;

        SphereMesh->OnComponentWake.AddDynamic(GameState, &APoolGameState::OnBallStartMoving);
        SphereMesh->OnComponentSleep.AddDynamic(GameState, &APoolGameState::OnBallStopMoving);

        SphereMesh->OnComponentBeginOverlap.AddDynamic(GameState, &APoolGameState::OnBallOverlap);
        SphereMesh->OnComponentEndOverlap.AddDynamic(GameState, &APoolGameState::OnBallEndOverlap);
    }
    // did not put into "else" cuz it is another piece of logic
    if (GetLocalRole() < ROLE_Authority)
    {
        SphereMesh->PutRigidBodyToSleep();
        SphereMesh->SetSimulatePhysics(false);
        SphereMesh->SetEnableGravity(false);
        SetActorEnableCollision(false);
    }
}


void ABall::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (GetLocalRole() < ROLE_Authority)
    {
        ClientSimulateFreeMovingBall();
    }
    else
    {
        ServerPhysicsState.Position = GetActorLocation();
        ServerPhysicsState.Rotation = GetActorRotation();
        ServerPhysicsState.Velocity = SphereMesh->GetComponentVelocity();
        ServerPhysicsState.TimeStamp = ABilliardistController::GetLocalTime();
    }
}

void ABall::OnRep_SmoothPhysicsState()
{
    for (int i = PROXY_STATE_ARRAY_SIZE_MAX - 1; i >= 1; i--)
    {
        ProxyStates[i] = ProxyStates[i - 1];
    }

    ProxyStates[0] = ServerPhysicsState;

    ProxyStateCountCurrent = FMath::Min(static_cast<uint8>(ProxyStateCountCurrent + 1), 
        PROXY_STATE_ARRAY_SIZE_MAX);

    if (ProxyStates[0].TimeStamp < ProxyStates[1].TimeStamp)
    {
        UE_LOG(LogOnlineGame, Error, TEXT("Timestamp inconsistent: %d should be greater than %d"), 
            ProxyStates[0].TimeStamp, ProxyStates[1].TimeStamp);
    }
}

void ABall::RemoveBallFromGame()
{
    auto Comp = Cast<UStaticMeshComponent>(GetRootComponent());
    Comp->SetSimulatePhysics(false);

    SetActorHiddenInGame(true);
}

void ABall::ReturnBallIntoGame()
{
    auto Comp = Cast<UStaticMeshComponent>(GetRootComponent());
    Comp->SetSimulatePhysics(true);

    SetActorHiddenInGame(false);
}

void ABall::ClientSimulateFreeMovingBall()
{
    auto MyPC = Cast<INetworkTimeProvider>(UGameplayStatics::GetPlayerController(GetWorld(), 0));

    if (MyPC == nullptr || !MyPC->IsTimeOffsetValid() || 0 == ProxyStateCountCurrent)
    {
        // We don't know yet know what the time is on the server yet so the timestamps
        // of the proxy states mean nothing; that or we simply don't have any proxy
        // states yet. Don't do any interpolation.
        SetActorLocationAndRotation(ServerPhysicsState.Position, ServerPhysicsState.Rotation);
    }
    else
    {
        uint64 InterpolationBackTime = 100;
        uint64 ExtrapolationLimit = 500;

        // This is the target playback time of the rigid body
        uint64 InterpolationTime = MyPC->GetNetworkTime() - InterpolationBackTime;

        // Use interpolation if the target playback time is present in the buffer
        if (ProxyStates[0].TimeStamp > InterpolationTime)
        {
            // Go through buffer and find correct state to play back
            for (int i = 0; i < ProxyStateCountCurrent; i++)
            {
                if (ProxyStates[i].TimeStamp <= InterpolationTime || i == ProxyStateCountCurrent - 1)
                {
                    // The state one slot newer (<100ms) than the best playback state
                    FSmoothPhysicsState rhs = ProxyStates[FMath::Max(i - 1, 0)];

                    // The best playback state (closest to 100 ms old (default time))
                    FSmoothPhysicsState lhs = ProxyStates[i];

                    // Use the time between the two slots to determine if interpolation is necessary
                    int64 length = (int64)(rhs.TimeStamp - lhs.TimeStamp);

                    double t = 0.0F;

                    // As the time difference gets closer to 100 ms t gets closer to 1 in
                    // which case rhs is only used
                    if (length > 1)
                        t = (double)(InterpolationTime - lhs.TimeStamp) / (double)length;

                    // if t=0 => lhs is used directly
                    FVector pos = FMath::Lerp(lhs.Position, rhs.Position, t);
                    FRotator rot = FMath::Lerp(lhs.Rotation, rhs.Rotation, t);

                    SetActorLocationAndRotation(pos, rot);

                    return;
                }
            }
        }
        // Use extrapolation
        else
        {
            FSmoothPhysicsState latest = ProxyStates[0];
            uint64 extrapolationLength = InterpolationTime - latest.TimeStamp;

            // Don't extrapolate for more than [extrapolationLimit] milliseconds
            if (extrapolationLength < ExtrapolationLimit)
            {
                FVector pos = latest.Position + latest.Velocity * ((float)extrapolationLength * 0.001f);
                FRotator rot = latest.Rotation;

                SetActorLocationAndRotation(pos, rot);
            }
            else
            {
                // Don't move. If we're this far away from the server, we must be pretty laggy.
                // Wait to catch up with the server.
            }
        }
    }
}

void ABall::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ABall, ServerPhysicsState);
}