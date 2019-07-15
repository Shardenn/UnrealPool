// Copyright 2019 Andrei Vikarchuk.

#include "EightBallSpawner.h"
#include "Pool.h"
#include "Objects/Table/TableEightBall.h"
#include "Objects/BallAmerican.h"

#include "UObject/ConstructorHelpers.h"

UEightBallSpawner::UEightBallSpawner()
{
    ConstructorHelpers::FObjectFinder<UBlueprint> BallBPObject(TEXT("/Game/Blueprints/Gameplay/BP_BallAmerican"));
    if (!ensure(BallBPObject.Object != nullptr)) return;

    BallClass = (UClass*)BallBPObject.Object->GeneratedClass;
}

void UEightBallSpawner::Spawn()
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