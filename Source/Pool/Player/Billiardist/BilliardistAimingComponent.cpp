// Copyright 2019 Andrei Vikarchuk.

#include "BilliardistAimingComponent.h"

#include "Pool.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "Math/InterpCurve.h"
#include "Kismet/KismetMathLibrary.h"

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

void UBilliardistAimingComponent::HandleFinishedAiming(AActor* const ActorToLookAt)
{
    UpdateHitStrengthRatio(-HitStrengthRatio);
    if (!SpringArm) { return; }
    
    ActorToLookAtWhileBlending = ActorToLookAt;

    StartingTransform.Location = ActorToLookAtWhileBlending->GetActorLocation();
    StartingTransform.Rotation = Cast<APawn>(GetOwner())->GetController()->GetControlRotation();

    FinalTransform.Location = GetDefaultCameraSpringWorldLocation();
    
    CurrentBlendingSpeed = BlendingStartingSpeed;

    BlendingState = EBlendingState::BlendingOut;
    //SpringArm->SetRelativeLocation(DefaultSpringArmLocation);
}

void UBilliardistAimingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    switch (BlendingState)
    {
        case EBlendingState::BlendingIn:
            break;
        case EBlendingState::BlendingOut:
            // A player can move while blending, so update final location all the time.
            FinalTransform.Location = GetDefaultCameraSpringWorldLocation();
            // keep final rotation updated, as we would like to watch at the launched ball
            FinalTransform.Rotation = (ActorToLookAtWhileBlending->GetActorLocation() - GetDefaultCameraSpringWorldLocation()).Rotation();

            InterpolationData CurrentTransform;
            CurrentTransform.Location = FMath::Lerp(StartingTransform.Location, FinalTransform.Location, BlendAlpha);
            CurrentTransform.Rotation = FMath::Lerp(StartingTransform.Rotation, FinalTransform.Rotation, BlendAlpha);

            Cast<APawn>(GetOwner())->GetController()->SetControlRotation(CurrentTransform.Rotation);
            SpringArm->SetWorldLocation(CurrentTransform.Location);

            CurrentBlendingSpeed = FMath::Clamp(CurrentBlendingSpeed - DeltaTime * BlendAlpha * BlendingSpeedChange,
                0.4f * BlendingStartingSpeed, BlendingStartingSpeed);
            BlendAlpha += DeltaTime * CurrentBlendingSpeed;

            if (BlendAlpha >= 1.f)
            {
                BlendingState = EBlendingState::None;
                BlendAlpha = 0.f;
            }
            break;
    }
}

