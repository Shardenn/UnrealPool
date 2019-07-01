// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Camera/CameraComponent.h"

#include "AimingComponent.generated.h"

UENUM(BlueprintType)
enum class FAimingComponentState : uint8
{
    GoingIn,
    GoingOut,
    InPlayer,
    InBall
};

UCLASS(ClassGroup = (Billiardist), meta = (BlueprintSpawnableComponent))
class POOL_API UAimingComponent : public UCameraComponent
{
    GENERATED_BODY()

public:
    // Sets default values for this component's properties
    UAimingComponent();

protected:
    // Called when the game starts
    virtual void BeginPlay() override;
public:
    // Called every frame
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


};
