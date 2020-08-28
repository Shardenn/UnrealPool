// Copyright 2019 Andrei Vikarchuk.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BallSpawner.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class POOL_API UBallSpawner : public UActorComponent
{
    GENERATED_BODY()

public:
    UBallSpawner();
    
    virtual TArray<class ABall*> Spawn();
    TArray<FVector> GetTriangleSpawnPoints(const FVector& HeadLocation,
        const FVector& RowsIncreaseDir, const FVector& ColsIncreaseDir,
        const float& BallDiameter);
protected:
    virtual void BeginPlay() override;

    float BallDiameter = 10; // some default so any error could be seen

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RowsNum = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<class ABall> BallClass;
};
