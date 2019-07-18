// Copyright 2019 Andrei Vikarchuk.

#include "BallSpawner.h"
#include "Pool.h"
#include "Table.h"
#include "../Ball.h"

#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

UBallSpawner::UBallSpawner()
{
    ConstructorHelpers::FObjectFinder<UBlueprint> BallBPObject(TEXT("/Game/Blueprints/Gameplay/BP_Ball"));
    if (!ensure(BallBPObject.Object != nullptr)) return;

    BallClass = (UClass*)BallBPObject.Object->GeneratedClass;
}

void UBallSpawner::BeginPlay()
{
    Super::BeginPlay();
}

void UBallSpawner::Spawn()
{
    ATable* Table = Cast<ATable>(GetOwner());
    if (!ensure(Table != nullptr)) return;

    UWorld* World = GetWorld();
    if (!ensure(World != nullptr)) return;

    USceneComponent* SpawnComponent = Table->FrontBallLocation;
    if (!ensure(SpawnComponent != nullptr)) return;

    FVector HeadBallLocation = SpawnComponent->GetComponentTransform().GetLocation();
    
    FVector RowsIncreaseDirection = -(SpawnComponent->GetForwardVector().GetSafeNormal());
    FVector ColumnsIncreaseDirection = -(SpawnComponent->GetRightVector().GetSafeNormal());

    FVector CurrentBallLocation = HeadBallLocation;
    ABall* Ball = World->SpawnActor<ABall>(BallClass, CurrentBallLocation, FRotator::ZeroRotator);
    BallDiameter = Ball->GetRootComponent()->Bounds.SphereRadius * 2;

    auto Locations = GetTriangleSpawnPoints(HeadBallLocation, RowsIncreaseDirection,
        ColumnsIncreaseDirection, BallDiameter);

    for (auto& Location : Locations)
    {
        World->SpawnActor<ABall>(BallClass, Location, FRotator::ZeroRotator);
    }
}

TArray<FVector> UBallSpawner::GetTriangleSpawnPoints(const FVector& HeadLocation,
    const FVector& RowsIncreaseDir, const FVector& ColsIncreaseDir,
    const float& InBallDiameter)
{
    // BallsCount in the triangle except the very first ball.
    // https://en.wikipedia.org/wiki/Arithmetic_progression
    TArray<FVector> Ret;
    uint32 BallsCount = (RowsNum - 1) * (2 + RowsNum) / 2;
    Ret.Init(HeadLocation, BallsCount);
    uint32 RetIndex = 0;

    FVector CurrentBallLocation = HeadLocation;
    for (uint8 i = 1; i < RowsNum; i++)
    {
        // when balls are arranged in triangle, they do not step directly Diameter from eatch other
        // in rows. They are a bit closer. After solving a simple geometry, we get that they
        // step from each other on  0.86602540378 of diameter (or sqrt(3) * Radius is the answer).
        CurrentBallLocation = HeadLocation + i * RowsIncreaseDir * (InBallDiameter * 0.8661f);
        CurrentBallLocation += i * ColsIncreaseDir * (InBallDiameter * 0.5f);

        for (uint8 j = 0; j <= i; j++)
        {
            Ret[RetIndex++] = CurrentBallLocation;
            CurrentBallLocation += -(ColsIncreaseDir) * (InBallDiameter);
        }
    }

    return Ret;
}
