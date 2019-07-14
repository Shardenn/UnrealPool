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

    for (int i = 1; i < RowsNum; i++)
    {
        CurrentBallLocation = HeadBallLocation + i * RowsIncreaseDirection * BallDiameter;
        CurrentBallLocation += i * ColumnsIncreaseDirection * (BallDiameter / 2.0f + BallsSpacing);

        for (int j = 0; j <= i; j++)
        {
            World->SpawnActor<ABall>(BallClass, CurrentBallLocation, FRotator::ZeroRotator);
            CurrentBallLocation += -(ColumnsIncreaseDirection)* (BallDiameter + BallsSpacing);
        }
    }
}