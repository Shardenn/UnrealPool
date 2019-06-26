// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Ball.generated.h"

UCLASS()
class POOL_API ABall : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    ABall();
    virtual void Tick(float DeltaTime) override;


protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, Meta = (DisplayName = "Static mesh"))
    UStaticMeshComponent* m_pSphereMesh = nullptr;

    UPROPERTY(EditAnywhere, Meta = (DisplayName = "Sphere collision"))
    USphereComponent* m_pSphereCollision = nullptr;

    UPROPERTY(EditAnywhere, Meta = (DisplayName = "Points score"))
    int32 m_Points = 1;

    UPROPERTY(EditAnywhere, Meta = (DisplayName = "Is it a cue ball"))
    bool m_bCueBall = false;

    UPROPERTY(EditAnywhere, Meta = (DisplayName = "SPawn location on start"))
    FVector m_SpawnLocation = FVector(0);

private:


};
