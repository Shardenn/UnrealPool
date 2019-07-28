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
    USplineComponent* GetSplinePath()                  { return SplinePath; }

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    USceneComponent* FrontBallLocation = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    USceneComponent* CueBallLocation = nullptr;

    virtual TArray<class ABall*> SpawnBalls();
protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    UStaticMeshComponent* TableMesh = nullptr;


    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    class UBoxComponent* BallRegistrator = nullptr;

    // Spline path for player movement along the table
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    class USplineComponent* SplinePath = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    class UBallSpawner* BallSpawner = nullptr;
};
