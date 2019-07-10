// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "BallRegistrator.generated.h"

UCLASS()
class POOL_API ABallRegistrator : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    ABallRegistrator();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, Meta=(DisplayName = "Box collider"))
    UBoxComponent* m_pBoxCollider = nullptr;
public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

};
