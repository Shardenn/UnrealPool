// Fill out your copyright notice in the Description page of Project Settings.

#include "Table.h"


// Sets default values
ATable::ATable()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;

    m_pTableMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Table mesh"));
    RootComponent = (USceneComponent*)m_pTableMesh;

    m_pFrontBallLocation = CreateDefaultSubobject<USceneComponent>(TEXT("Front ball location"));
    m_pBallRegistrator = CreateDefaultSubobject<UBoxComponent>(TEXT("Falling balls registrator"));

    float zBound = RootComponent->Bounds.BoxExtent.Z; // size of the table for default ball spawn location
    m_pFrontBallLocation->SetRelativeLocation(FVector(0, 0, 2 * zBound)); // spawn above the table if we did not define otherwise

    m_pSplinePath = CreateDefaultSubobject<USplineComponent>(TEXT("Spline player path"));
    m_pSplinePath->SetIsReplicated(true);
}

// Called when the game starts or when spawned
void ATable::BeginPlay()
{
    Super::BeginPlay();

    if (!m_pTableMesh || !m_pFrontBallLocation)
    {
        UE_LOG(LogTemp, Error, TEXT("Object %s has either no table mesh or fron ball location component."), *GetName());
        return;
    }

    if (!m_pSplinePath)
    {
        UE_LOG(LogTemp, Error, TEXT("Object %s has has no spline component for player path."), *GetName());
        return;
    }

}

// Called every frame
void ATable::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

}

