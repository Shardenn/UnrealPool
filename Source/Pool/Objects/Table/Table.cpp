// Copyright 2019 Andrei Vikarchuk.

#include "Table.h"
#include "BallSpawner.h"
#include "Pool.h"

#include "Components/BoxComponent.h"
#include "Components/SplineComponent.h"

// Sets default values
ATable::ATable()
{
    bReplicates = true;

    TableMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Table mesh"));
    RootComponent = (USceneComponent*)TableMesh;

    FrontBallLocation = CreateDefaultSubobject<USceneComponent>(TEXT("Front ball location"));
    BallRegistrator = CreateDefaultSubobject<UBoxComponent>(TEXT("Falling balls registrator"));

    float zBound = RootComponent->Bounds.BoxExtent.Z; // size of the table for default ball spawn location
    FrontBallLocation->SetRelativeLocation(FVector(0, 0, 2 * zBound)); // spawn above the table if we did not define otherwise

    SplinePath = CreateDefaultSubobject<USplineComponent>(TEXT("Spline player path"));
    SplinePath->SetIsReplicated(true);

    BallSpawner = CreateDefaultSubobject<UBallSpawner>(TEXT("Ball spawner"));
}

void ATable::SpawnBalls()
{
    BallSpawner->Spawn();
}

// Called when the game starts or when spawned
void ATable::BeginPlay()
{
    Super::BeginPlay();
}
