// Copyright 2019 Andrei Vikarchuk.

#include "EightBallSpawner.h"
#include "Pool.h"
#include "Objects/Table/TableEightBall.h"
#include "Objects/BallAmerican.h"

#include "UObject/ConstructorHelpers.h"
#include "Algo/Reverse.h"
#include "Kismet/KismetMathLibrary.h"

UEightBallSpawner::UEightBallSpawner()
{
    /*ConstructorHelpers::FObjectFinder<UBlueprint> BallBPObject(TEXT("/Game/Blueprints/Gameplay/BP_BallAmerican.BP_BallAmerican_C"));
    if (!ensure(BallBPObject.Object != nullptr)) return;

    BallClass = (UClass*)BallBPObject.Object->GeneratedClass;*/
}

TArray<class ABall*> UEightBallSpawner::Spawn()
{
    TArray<ABall*> Balls;
    
    if (!BallClass || !BallClass.Get())
    {
        UE_LOG(LogPool, Warning, TEXT("No ball class assigned for BallSpawner %s"), *GetName());
        return Balls;
    }

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
    // remove 8ball, we have special plans for it
    BallsNum.RemoveAt(7);

    ABallAmerican* Ball = World->SpawnActor<ABallAmerican>(BallClass, CurrentBallLocation, UKismetMathLibrary::RandomRotator(true));
    uint8 BallNum = GetRandomBallNum(BallsNum, true);
    Ball->SetBallNumber(BallNum);
    Balls.Add(Ball);

    BallDiameter = Ball->GetRootComponent()->Bounds.SphereRadius * 2;

    auto Locations = GetTriangleSpawnPoints(HeadBallLocation, RowsIncreaseDirection,
        ColumnsIncreaseDirection, BallDiameter);
    // for the rule "Different ball types in the corners"
    // it is easier to reverse the array
    Algo::Reverse(Locations);

    FBallType CornerBallType = FBallType::NotInitialized;
    uint8 CornerBallNum = 0;
    for (uint8 i = 0; i < Locations.Num(); i++)
    {
        auto Location = Locations[i];
        Ball = World->SpawnActor<ABallAmerican>(BallClass, Location, UKismetMathLibrary::RandomRotator(true));
        
        // 8ball position
        if (i == 10)
        {
            BallNum = 8;
            Ball->SetBallNumber(BallNum);
            Balls.Add(Ball);
            continue;
        }

        BallNum = GetRandomBallNum(BallsNum);

        // regenerate 2nd corner ball num until it is different
        // type from the 1st corner ball
        if (i == 4) // the second corner ball check
            while ((8 - BallNum) * (8 - CornerBallNum) > 0)
                BallNum = GetRandomBallNum(BallsNum);

        Ball->SetBallNumber(BallNum);
        BallsNum.Remove(BallNum);
        Balls.Add(Ball);

        if (CornerBallType == FBallType::NotInitialized)
        {
            CornerBallType = Ball->GetType();
            CornerBallNum = BallNum;
        }

    }

    // Spawn cue ball
    SpawnComponent = Table->CueBallLocation;
    FVector Location = SpawnComponent->GetComponentTransform().GetLocation();
    Ball = World->SpawnActor<ABallAmerican>(BallClass, Location, FRotator::ZeroRotator);
    Ball->SetBallNumber(16);
    Balls.Add(Ball);

    return Balls;
}

uint8 UEightBallSpawner::GetRandomBallNum(TArray<uint8>& AvailableNumbers, bool bRemoveNumber)
{
    uint8 NumberIndex = FMath::RandHelper(AvailableNumbers.Num());
    uint8 BallNum = AvailableNumbers[NumberIndex];
    if (bRemoveNumber)
        AvailableNumbers.RemoveAt(NumberIndex);
    
    return BallNum;
}
