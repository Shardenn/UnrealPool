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
}

// Called when the game starts or when spawned
void ABall::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        const APoolGameState* GameState = Cast<APoolGameState>(UGameplayStatics::GetGameState(GetWorld()));
        if (!ensure(GameState != nullptr)) return;

        SphereMesh->OnComponentWake.AddDynamic(GameState, &APoolGameState::OnBallStartMoving);
        SphereMesh->OnComponentSleep.AddDynamic(GameState, &APoolGameState::OnBallStopMoving);

        SphereMesh->OnComponentBeginOverlap.AddDynamic(GameState, &APoolGameState::OnBallOverlap);
        SphereMesh->OnComponentEndOverlap.AddDynamic(GameState, &APoolGameState::OnBallEndOverlap);

        SphereMesh->SetSimulatePhysics(true);
        SphereMesh->SetEnableGravity(true);

        SphereMesh->BodyInstance.bGenerateWakeEvents = true;
        //SphereMesh->SetMassOverrideInKg(NAME_None, 0.2);
        SphereMesh->SetAngularDamping(0.6);
        SphereMesh->SetLinearDamping(0.2);

        SphereMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        SphereMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
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
        ClientTick(DeltaTime);
    }
    else
    {
        ServerPhysicsState.Position = GetActorLocation();
        ServerPhysicsState.Rotation = GetActorRotation().Quaternion();
        ServerPhysicsState.Velocity = SphereMesh->GetComponentVelocity();
        ServerPhysicsState.TimeStamp = ABilliardistController::GetLocalTime();
    }

}

void ABall::ClientTick(float DeltaTime)
{
    ClientTimeSinceUpdate += DeltaTime;

    if (ClientTimeBetweenLastUpdates < KINDA_SMALL_NUMBER) return;
    
    FHermiteCubicSpline Spline = CreateSpline();
    const float Ratio = ClientTimeSinceUpdate / ClientTimeBetweenLastUpdates;

    InterpolateVelocity(Spline, Ratio);
    InterpolateLocation(Spline, Ratio);
    InterpolateRotation(Ratio);
}

FHermiteCubicSpline ABall::CreateSpline()
{
    FHermiteCubicSpline Spline;
    Spline.StartLocation = ClientStartTransform.GetLocation();
    Spline.TargetLocation = ServerPhysicsState.Position;
    Spline.StartDerivative = ClientStartVelocity * VelocityToDerivative();
    Spline.TargetDerivative = ServerPhysicsState.Velocity * VelocityToDerivative();

    return Spline;
}

void ABall::InterpolateLocation(const FHermiteCubicSpline& Spline, const float Ratio)
{
    const FVector NewLocation = Spline.InterpolateLocation(Ratio);
    SetActorLocation(NewLocation);
}

void ABall::InterpolateRotation(const float Ratio)
{
    const FQuat StartRotation = ClientStartTransform.GetRotation();
    const FQuat TargetRotation = ServerPhysicsState.Rotation;
    const FQuat NewRotation = FQuat::Slerp(StartRotation, TargetRotation, Ratio);
    SetActorRotation(NewRotation);
}

void ABall::InterpolateVelocity(const FHermiteCubicSpline& Spline, const float Ratio)
{
    const FVector NewDerivative = Spline.InterpolateDerivative(Ratio);
    ClientStartVelocity = NewDerivative / VelocityToDerivative();
}

void ABall::OnRep_SmoothPhysicsState()
{
    ClientTimeBetweenLastUpdates = ClientTimeSinceUpdate;
    ClientTimeSinceUpdate = 0;

    ClientStartTransform = GetActorTransform();
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

void ABall::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ABall, ServerPhysicsState);
}