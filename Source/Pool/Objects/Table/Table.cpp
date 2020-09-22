// Copyright 2019 Andrei Vikarchuk.

#include "Table.h"
#include "Pool.h"

#include "BallSpawner.h"
#include "Components/ActivePlayArea.h"
#include "Components/BallRegistrator.h"
#include "Components/InitialBallPlacementArea.h"
// TODO doubtful include
#include "Objects/Ball.h"
#include "GameplayLogic/PoolGameMode.h"

#include "Components/SplineComponent.h"
#include "Components/CapsuleComponent.h"

// Sets default values
ATable::ATable()
{
    bReplicates = true;

    TableMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Table mesh"));
    RootComponent = (USceneComponent*)TableMesh;

    TableMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_TableTraceChannel, ECollisionResponse::ECR_Block);

    FrontBallLocation = CreateDefaultSubobject<USceneComponent>(TEXT("Front ball location"));
    CueBallLocation = CreateDefaultSubobject<USceneComponent>(TEXT("Cue ball location"));

    BallRegistrator = CreateDefaultSubobject<UBallRegistrator>(TEXT("Falling balls registrator"));
    ActivePlayArea = CreateDefaultSubobject<UActivePlayArea>(TEXT("Dropped balls registrator"));
    InitialBallPlacementArea = CreateDefaultSubobject<UInitialBallPlacementArea>(TEXT("Initial cue ball placement area"));

    float zBound = RootComponent->Bounds.BoxExtent.Z; // size of the table for default ball spawn location
    FrontBallLocation->SetRelativeLocation(FVector(0, 0, 2 * zBound)); // spawn above the table if we did not define otherwise
    CueBallLocation->SetRelativeLocation(FVector(0, 0, 2 * zBound)); // spawn above the table if we did not define otherwise

    SplinePath = CreateDefaultSubobject<USplineComponent>(TEXT("Spline player path"));
    SplinePath->SetIsReplicated(true);

    BallSpawner = CreateDefaultSubobject<UBallSpawner>(TEXT("Ball spawner"));
}

// Called when the game starts or when spawned
void ATable::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        APoolGameMode* GM = Cast<APoolGameMode>(GetWorld()->GetAuthGameMode());
        if (!ensure(GM != nullptr)) return;

        GM->OnFrameRestart.AddDynamic(this, &ATable::SpawnBalls);
    }
}

//TArray<class ABall*> ATable::SpawnBalls()
void ATable::SpawnBalls()
{
    for (auto& Ball : SpawnedBalls)
    {
        Ball->Destroy();
    }
    SpawnedBalls.Empty();

    SpawnedBalls = BallSpawner->Spawn();
}

