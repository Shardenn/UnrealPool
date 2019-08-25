// Copyright 2019 Andrei Vikarchuk.

#include "BilliardistAimingComponent.h"

#include "GameFramework/Actor.h"
#include "GameFramework/SpringArmComponent.h"

UBilliardistAimingComponent::UBilliardistAimingComponent()
{
    PrimaryComponentTick.bCanEverTick = true;

    //SpringArm = GetOwner()->FindComponentByClass<USpringArmComponent>();
}

void UBilliardistAimingComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UBilliardistAimingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

