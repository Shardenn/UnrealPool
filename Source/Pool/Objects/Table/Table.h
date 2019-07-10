// Fill out your copyright notice in the Description page of Project Settings.

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
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintPure, meta = (DisplayName = "Get Spline Path"))
    USplineComponent* GetSplinePath()                  { return SplinePath; }
protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    UStaticMeshComponent* TableMesh = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    USceneComponent* FrontBallLocation = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    class UBoxComponent* BallRegistrator = nullptr;

    // Spline path for player movement along the table
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    class USplineComponent* SplinePath = nullptr;
};
