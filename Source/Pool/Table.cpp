// Fill out your copyright notice in the Description page of Project Settings.

#include "Table.h"

// Sets default values
ATable::ATable()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;

    TableMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Table mesh"));
    RootComponent = (USceneComponent*)TableMesh;

    FrontBallLocation = CreateDefaultSubobject<USceneComponent>(TEXT("Front ball location"));
    BallRegistrator = CreateDefaultSubobject<UBoxComponent>(TEXT("Falling balls registrator"));

    float zBound = RootComponent->Bounds.BoxExtent.Z; // size of the table for default ball spawn location
    FrontBallLocation->SetRelativeLocation(FVector(0, 0, 2 * zBound)); // spawn above the table if we did not define otherwise

    SplinePath = CreateDefaultSubobject<USplineComponent>(TEXT("Spline player path"));
    SplinePath->SetIsReplicated(true);
}

// Called when the game starts or when spawned
void ATable::BeginPlay()
{
    Super::BeginPlay();
}

// Called every frame
void ATable::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

