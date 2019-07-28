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

TArray<class ABall*> UEightBallSpawner::Spawn()
{
    TArray<ABall*> Balls;
    
    ATable* Table = Cast<ATable>(GetOwner());
    if (!ensure(Table != nullptr)) return Balls;

    UWorld* World = GetWorld();
    if (!ensure(World != nullptr)) return Balls;

    USceneComponent* SpawnComponent = Table->FrontBallLocation;
    if (!ensure(SpawnComponent != nullptr)) return Balls;

    FVector HeadBallLocation = SpawnComponent->GetComponentTransform().GetLocation();

    FVector RowsIncreaseDirection = -(SpawnComponent->GetForwardVector().GetSafeNormal());
    FVector ColumnsIncreaseDirection = -(SpawnComponent->GetRightVector().GetSafeNormal());

    FVector CurrentBallLocation = HeadBallLocation;
    
    // Stores balls num for random ball init and remembering
    // which balls are already inited
    TArray<uint8> BallsNum;
    BallsNum.Init(0, 15);
    for (uint8 i = 0; i < BallsNum.Num(); i++)
    {
        BallsNum[i] = i + 1;
    }

    ABallAmerican* Ball = World->SpawnActor<ABallAmerican>(BallClass, CurrentBallLocation, FRotator::ZeroRotator);

    uint8 NumberIndex = FMath::RandHelper(BallsNum.Num());
    Ball->SetBallNumber(BallsNum[NumberIndex]);
    Balls.Add(Ball);

    BallsNum.RemoveAt(NumberIndex);

    BallDiameter = Ball->GetRootComponent()->Bounds.SphereRadius * 2;

    auto Locations = GetTriangleSpawnPoints(HeadBallLocation, RowsIncreaseDirection,
        ColumnsIncreaseDirection, BallDiameter);

    for (uint8 i = 0; i < Locations.Num(); i++)
    {
        auto Location = Locations[i];
        Ball = World->SpawnActor<ABallAmerican>(BallClass, Location, FRotator::ZeroRotator);
        
        uint8 NumberIndex = FMath::RandHelper(BallsNum.Num());
        Ball->SetBallNumber(BallsNum[NumberIndex]);
        Balls.Add(Ball);

        BallsNum.RemoveAt(NumberIndex);
    }

    // Spawn cue ball
    SpawnComponent = Table->CueBallLocation;
    FVector Location = SpawnComponent->GetComponentTransform().GetLocation();
    Ball = World->SpawnActor<ABallAmerican>(BallClass, Location, FRotator::ZeroRotator);
    Ball->SetBallNumber(16);
    Balls.Add(Ball);

    return Balls;
}