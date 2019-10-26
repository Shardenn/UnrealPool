// Copyright 2019 Andrei Vikarchuk.

#include "BilliardistAimingComponent.h"

#include "Pool.h"
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

void UBilliardistAimingComponent::UpdateHitStrengthRatio(float Delta)
{
    HitStrengthRatio = FMath::Clamp(HitStrengthRatio + Delta * HitStrengthadjustmentSpeed, 0.f, 1.f);
    HitStrength = MaxAcceptableHitStrength * HitStrengthRatio;
}

void UBilliardistAimingComponent::AdjustZoom(float Delta)
{
    SpringArm->TargetArmLength = FMath::Clamp(SpringArm->TargetArmLength + Delta * ZoomAdjustementSpeed,
        ZoomSpringArmLengthMin, ZoomSpringArmLengthMax);
}

void UBilliardistAimingComponent::HandleStartedAiming(const FVector& AimedAt)
{
    if (!SpringArm) { return; }
    // TODO make interpolation
    SpringArm->SetWorldLocation(AimedAt);
}

void UBilliardistAimingComponent::HandleFinishedAiming()
{
    UpdateHitStrengthRatio(-HitStrengthRatio);
    if (!SpringArm) { return; }
    // TODO make interpolation
    SpringArm->SetRelativeLocation(DefaultSpringArmLocation);
}

void UBilliardistAimingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

