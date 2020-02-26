// Copyright 2019 Andrei Vikarchuk.

#include "BilliardistAimingComponent.h"

#include "Pool.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Math/Quat.h"

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
    LastSpringArmLength = SpringArm->TargetArmLength;
}

void UBilliardistAimingComponent::HandleStartedAiming(const FVector& AimedAt)
{
    if (!SpringArm) { return; }

    StartingTransform.Location = SpringArm->GetComponentLocation();
    StartingTransform.Rotation = Cast<APawn>(GetOwner())->GetController()->GetControlRotation().Quaternion();

    // TODO AimedAt should affect the rotation, not the location
    FinalTransform.Location = AimedAt;
    FinalTransform.Rotation = (AimedAt - GetDefaultCameraSpringWorldLocation()).Rotation().Quaternion();;

    BlendingState = EBlendingState::BlendingIn;
}

void UBilliardistAimingComponent::HandleFinishedAiming(AActor* const ActorToLookAt)
{
    UpdateHitStrengthRatio(-HitStrengthRatio);
    if (!SpringArm) { return; }
    
    ActorToLookAtWhileBlending = ActorToLookAt;

    StartingTransform.Location = SpringArm->GetComponentLocation();
    StartingTransform.Rotation = Cast<APawn>(GetOwner())->GetController()->GetControlRotation().Quaternion();

    FinalTransform.Location = GetDefaultCameraSpringWorldLocation();
    FinalTransform.Rotation = (ActorToLookAtWhileBlending->GetActorLocation() - GetDefaultCameraSpringWorldLocation()).ToOrientationQuat();

    BlendingState = EBlendingState::BlendingOut;
}

void UBilliardistAimingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    float EasedAlpha = 0.f;
    float SpringArmAlpha = 0.f;

    switch (BlendingState)
    {
        case EBlendingState::BlendingIn:
            EasedAlpha = UKismetMathLibrary::Ease(0.f, 1.f, BlendAlpha, EEasingFunc::ExpoOut);

            CurrentTransform.Location = FMath::Lerp(StartingTransform.Location, FinalTransform.Location, EasedAlpha);
            CurrentTransform.Rotation = FMath::Lerp(StartingTransform.Rotation, FinalTransform.Rotation, EasedAlpha);

            Cast<APawn>(GetOwner())->GetController()->SetControlRotation(CurrentTransform.Rotation.Rotator());
            SpringArm->SetWorldLocation(CurrentTransform.Location);

            SpringArm->TargetArmLength = FMath::Lerp(0.f, LastSpringArmLength, EasedAlpha);

            BlendAlpha += DeltaTime * BlendingSpeed;

            if (BlendAlpha >= 1.f)
            {
                BlendingState = EBlendingState::None;
                BlendAlpha = 1.f;

                SpringArm->TargetArmLength = LastSpringArmLength;
                SpringArm->SetWorldLocation(FinalTransform.Location);
            }
            break;

        case EBlendingState::BlendingOut:
            EasedAlpha = UKismetMathLibrary::Ease(1.f, 0.f, BlendAlpha, EEasingFunc::ExpoOut);
            // A player can move while blending, so update final location all the time.
            FinalTransform.Location = GetDefaultCameraSpringWorldLocation();
            // keep final rotation updated, as we would like to watch at the launched ball
            FinalTransform.Rotation = (ActorToLookAtWhileBlending->GetActorLocation() - GetDefaultCameraSpringWorldLocation()).ToOrientationQuat();

            CurrentTransform.Location = FMath::Lerp(StartingTransform.Location, FinalTransform.Location, EasedAlpha);
            CurrentTransform.Rotation = FMath::Lerp(StartingTransform.Rotation, FinalTransform.Rotation, EasedAlpha);

            Cast<APawn>(GetOwner())->GetController()->SetControlRotation(CurrentTransform.Rotation.Rotator());
            SpringArm->SetWorldLocation(CurrentTransform.Location);

            SpringArm->TargetArmLength = FMath::Lerp(LastSpringArmLength, 0.f, EasedAlpha);

            BlendAlpha -= DeltaTime * BlendingSpeed;

            if (BlendAlpha <= 0.f)
            {
                BlendingState = EBlendingState::None;
                BlendAlpha = 0.f;

                SpringArm->TargetArmLength = 0.f;
                SpringArm->SetWorldLocation(FinalTransform.Location);
            }
            break;
    }
}
