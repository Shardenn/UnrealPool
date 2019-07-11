// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Ball.generated.h"

UCLASS()
class POOL_API ABall : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    ABall();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int32 Points = 1;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    bool CueBall = false;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FVector SpawnLocation = FVector(0);

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (DisplayName = "Static mesh"))
    UStaticMeshComponent* SphereMesh;

private:
    class APoolGameState* GameState = nullptr;
};
