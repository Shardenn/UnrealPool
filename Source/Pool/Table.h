// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
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

    UPROPERTY(EditAnywhere, meta = (DisplayName = "Table mesh"))
    UStaticMeshComponent* m_pTableMesh = nullptr;

    UPROPERTY(EditAnywhere, Meta = (DisplayName = "Triangle front ball location"))
    USceneComponent* m_pFrontBallLocation = nullptr;

    UPROPERTY(EditAnywhere, Meta = (DisplayName = "Ball fall registrator"))
    UBoxComponent* m_pBallRegistrator = nullptr;
public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;



};
