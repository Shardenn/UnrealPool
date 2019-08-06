// Copyright 2019 Andrei Vikarchuk.

#include "Table.h"
#include "Pool.h"
#include "BallSpawner.h"
// TODO doubtful include
#include "Objects/Ball.h"
#include "AmericanPool/PoolGameMode.h"

#include "Components/BoxComponent.h"
#include "Components/SplineComponent.h"

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

// Sets default values
ATable::ATable()
{
    bReplicates = true;

    TableMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Table mesh"));
    RootComponent = (USceneComponent*)TableMesh;

    FrontBallLocation = CreateDefaultSubobject<USceneComponent>(TEXT("Front ball location"));
    CueBallLocation = CreateDefaultSubobject<USceneComponent>(TEXT("Cue ball location"));

    BallRegistrator = CreateDefaultSubobject<UBoxComponent>(TEXT("Falling balls registrator"));
    BallRegistrator->SetRelativeLocation(FVector::ZeroVector);

    float zBound = RootComponent->Bounds.BoxExtent.Z; // size of the table for default ball spawn location
    FrontBallLocation->SetRelativeLocation(FVector(0, 0, 2 * zBound)); // spawn above the table if we did not define otherwise
    CueBallLocation->SetRelativeLocation(FVector(0, 0, 2 * zBound)); // spawn above the table if we did not define otherwise

    SplinePath = CreateDefaultSubobject<USplineComponent>(TEXT("Spline player path"));
    SplinePath->SetRelativeLocation(FVector::ZeroVector);
    SplinePath->SetIsReplicated(true);

    BallSpawner = CreateDefaultSubobject<UBallSpawner>(TEXT("Ball spawner"));
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

