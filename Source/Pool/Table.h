// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/SplineComponent.h"
#include "Table.generated.h"

UCLASS()
class POOL_API ATable : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    ATable();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Table mesh"))
    UStaticMeshComponent* m_pTableMesh = nullptr;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (DisplayName = "Triangle front ball location"))
    USceneComponent* m_pFrontBallLocation = nullptr;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (DisplayName = "Ball fall registrator"))
    UBoxComponent* m_pBallRegistrator = nullptr;

    // Spline path for player movement along the table
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (DisplayName = "Spline player path"))
    USplineComponent* m_pSplinePath = nullptr;
public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintPure, meta = (DisplayName = "Get Spline Path"))
    USplineComponent* GetSplinePath()                                        { return m_pSplinePath; }

};
