// Copyright 2019 Andrei Vikarchuk.

#include "BilliardistAimingComponent.h"

#include "GameFramework/Pawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "Math/InterpCurve.h"

UBilliardistAimingComponent::UBilliardistAimingComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UBilliardistAimingComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UBilliardistAimingComponent::Initialize(USpringArmComponent* InSpringArm)
{
    SpringArm = InSpringArm;
    if (SpringArm)
        DefaultSpringArmLocation = SpringArm->GetRelativeTransform().GetLocation();
}

void UBilliardistAimingComponent::UpdateHitStrength(float Delta)
{
    HitStrength = FMath::Clamp(HitStrength + Delta, 0.f, MaxAcceptableHitStrength);
}

void UBilliardistAimingComponent::HandleStartedAiming(const FVector& AimedAt)
{
    if (!SpringArm) { return; }
    // TODO make interpolation
    SpringArm->SetWorldLocation(AimedAt);
}

void UBilliardistAimingComponent::HandleFinishedAiming()
{
    if (!SpringArm) { return; }
    // TODO make interpolation
    SpringArm->SetRelativeLocation(DefaultSpringArmLocation);
}

void UBilliardistAimingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

