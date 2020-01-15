// Copyright 2019 Andrei Vikarchuk.

#include "BilliardistAimingComponent.h"

#include "Pool.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"
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

    StartingTransform.Location = SpringArm->GetComponentLocation();
    StartingTransform.Rotation = Cast<APawn>(GetOwner())->GetController()->GetControlRotation();

    // TODO AimedAt should affect the rotation, not the location
    FinalTransform.Location = AimedAt;
    FinalTransform.Rotation = (AimedAt - GetDefaultCameraSpringWorldLocation()).Rotation();

    CurrentBlendingSpeed = BlendingStartingSpeed;

    BlendingState = EBlendingState::BlendingIn;
}

void UBilliardistAimingComponent::HandleFinishedAiming(AActor* const ActorToLookAt)
{
    UpdateHitStrengthRatio(-HitStrengthRatio);
    if (!SpringArm) { return; }
    
    ActorToLookAtWhileBlending = ActorToLookAt;

    StartingTransform.Location = SpringArm->GetComponentLocation();
    StartingTransform.Rotation = Cast<APawn>(GetOwner())->GetController()->GetControlRotation();

    FinalTransform.Location = GetDefaultCameraSpringWorldLocation();
    FinalTransform.Rotation = (ActorToLookAtWhileBlending->GetActorLocation() - GetDefaultCameraSpringWorldLocation()).Rotation();

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
                BlendAlpha = 1.f;
            }
            break;
        case EBlendingState::BlendingOut:
            // A player can move while blending, so update final location all the time.
            FinalTransform.Location = GetDefaultCameraSpringWorldLocation();
            // keep final rotation updated, as we would like to watch at the launched ball
            FinalTransform.Rotation = (ActorToLookAtWhileBlending->GetActorLocation() - GetDefaultCameraSpringWorldLocation()).Rotation();

            CurrentTransform.Location = FMath::Lerp(StartingTransform.Location, FinalTransform.Location, 1 - BlendAlpha);
            CurrentTransform.Rotation = FMath::Lerp(StartingTransform.Rotation, FinalTransform.Rotation, 1 - BlendAlpha);

            Cast<APawn>(GetOwner())->GetController()->SetControlRotation(CurrentTransform.Rotation);
            SpringArm->SetWorldLocation(CurrentTransform.Location);

            CurrentBlendingSpeed = FMath::Clamp(CurrentBlendingSpeed - DeltaTime * BlendAlpha * BlendingSpeedChange,
                0.4f * BlendingStartingSpeed, BlendingStartingSpeed);
            BlendAlpha -= DeltaTime * CurrentBlendingSpeed;

            if (BlendAlpha <= 0.f)
            {
                BlendingState = EBlendingState::None;
                BlendAlpha = 0.f;
            }
            break;
    }
}

