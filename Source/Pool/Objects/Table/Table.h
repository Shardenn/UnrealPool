// Copyright 2019 Andrei Vikarchuk.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Table.generated.h"

UCLASS()
class POOL_API ATable : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    ATable();

    UFUNCTION(BlueprintPure, meta = (DisplayName = "Get Spline Path"))
    class USplineComponent* GetSplinePath()                  { return SplinePath; }

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    USceneComponent* FrontBallLocation = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    USceneComponent* CueBallLocation = nullptr;

    //virtual TArray<class ABall*> SpawnBalls();
    UFUNCTION()
    void SpawnBalls();
protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    UStaticMeshComponent* TableMesh = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    class UBallRegistrator* BallRegistrator = nullptr;

    // zone of play area. If a ball leaves this area,
    // it is considered as dropped
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    class UActivePlayArea* ActivePlayArea = nullptr;

    // Spline path for player movement along the table
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    class USplineComponent* SplinePath = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    class UBallSpawner* BallSpawner = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    class UInitialBallPlacementArea* InitialBallPlacementArea = nullptr;

private:
    // pointers to spawned balls.
    // Needed to destroy the balls on a frame end
    TArray<class ABall*> SpawnedBalls;
};
