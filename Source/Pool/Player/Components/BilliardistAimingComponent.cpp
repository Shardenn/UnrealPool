// Copyright 2019 Andrei Vikarchuk.

#include "BilliardistAimingComponent.h"

#include "Pool.h"

#include "Objects/Cue.h"

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

    MyOwner = Cast<APawn>(GetOwner());

    if (!ensure(MyOwner != nullptr)) return;

    bInControlOfPawn = MyOwner->GetLocalRole() == ROLE_AutonomousProxy ||
        MyOwner->GetRemoteRole() == ROLE_SimulatedProxy;
}


void UBilliardistAimingComponent::GetCueLocationAndRotation(FVector& Location, FQuat& Rotation)
{
    FVector LookDir = MyOwner->GetController()->GetControlRotation().Vector();
    //LookDir.Z = 0;
    Location = FinalTransform.Location + (-LookDir * (BallRadius + CuePlacementThreshold));
    
    FRotator StartRotation = MyOwner->GetController()->GetControlRotation();
    StartRotation.Pitch += CueRotationOffsetFromControlRotation;

    Rotation = StartRotation.Quaternion();
}

void UBilliardistAimingComponent::UpdateCueLocation(const FVector& AimOffset /*= FVector(0,0,0)*/, const FQuat& RotationOffset)
{
    if (!bInControlOfPawn) return;

    FVector CueLocation;
    FQuat CueRotation;
    GetCueLocationAndRotation(CueLocation, CueRotation);
    Server_MoveCue(CueLocation + AimOffset, CueRotation);
}


void UBilliardistAimingComponent::OnCueOverlap(UPrimitiveComponent* OverlappedComponent, 
    AActor* OtherActor, 
    UPrimitiveComponent* OtherComp, 
    int32 OtherBodyIndex, 
    bool bFromSweep, 
    const FHitResult& SweepResult)
{

}

void UBilliardistAimingComponent::Server_DestroyCue_Implementation()
{
    if (Cue)
        Cue->Destroy();
}

void UBilliardistAimingComponent::Server_MoveCue_Implementation(const FVector& Location, const FQuat& Rotation)
{
    if (!Cue)
    {
        UE_LOG(LogTemp, Warning, TEXT("No cue created, but tried to move it."));
        return;
    }

    Cue->SetActorLocation(Location);
    Cue->SetActorRotation(Rotation);
}

void UBilliardistAimingComponent::Server_CreateCue_Implementation(const FVector& Location, const FQuat& Rotation)
{
    if (!IsValid(CueClass))
    {
        UE_LOG(LogTemp, Warning, TEXT("No correct cue class is set in aiming component."));
        return;
    }

    Cue = GetWorld()->SpawnActor<ACue>(CueClass, Location, FRotator(Rotation));
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

    CueOffsetMultiplier = MaxCueOffsetMultiplier * HitStrengthRatio;
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

    const FVector LookDir = MyOwner->GetController()->GetControlRotation().Vector();
    FVector CueCreationLocation;
    FQuat CueCreationRotation;
    GetCueLocationAndRotation(CueCreationLocation, CueCreationRotation);

    Server_CreateCue(CueCreationLocation, CueCreationRotation);
}

void UBilliardistAimingComponent::HandleFinishedAiming(AActor* const ActorToLookAt)
{
    UpdateHitStrengthRatio(-HitStrengthRatio);
    if (!SpringArm) { return; }
    
    ActorToLookAtWhileBlending = ActorToLookAt;

    StartingTransform.Location = SpringArm->GetComponentLocation();
    StartingTransform.Rotation = MyOwner->GetController()->GetControlRotation().Quaternion();

    FinalTransform.Location = GetDefaultCameraSpringWorldLocation();
    FinalTransform.Rotation = (ActorToLookAtWhileBlending->GetActorLocation() - GetDefaultCameraSpringWorldLocation()).ToOrientationQuat();

    BlendingState = EBlendingState::BlendingOut;

    Server_DestroyCue();
}

void UBilliardistAimingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    float EasedAlpha = 0.f;
    float SpringArmAlpha = 0.f;
    FVector AimOffset;

    switch (BlendingState)
    {
        case EBlendingState::BlendedIn:
            if (Cue)
            {
                const FVector Forward = Cue->GetActorForwardVector();
                AimOffset = -Forward * CueOffsetMultiplier;
                const FVector Up = Cue->GetActorUpVector();
                AimOffset += -Up * CueDownOffsetMultiplier;
            }
            UpdateCueLocation(AimOffset);
            break;
        case EBlendingState::BlendingIn:
            EasedAlpha = UKismetMathLibrary::Ease(0.f, 1.f, BlendAlpha, EEasingFunc::ExpoOut);

            CurrentTransform.Location = FMath::Lerp(StartingTransform.Location, FinalTransform.Location, EasedAlpha);
            CurrentTransform.Rotation = FMath::Lerp(StartingTransform.Rotation, FinalTransform.Rotation, EasedAlpha);

            MyOwner->GetController()->SetControlRotation(CurrentTransform.Rotation.Rotator());
            SpringArm->SetWorldLocation(CurrentTransform.Location);

            SpringArm->TargetArmLength = FMath::Lerp(0.f, LastSpringArmLength, EasedAlpha);

            BlendAlpha += DeltaTime * BlendingSpeed;

            if (BlendAlpha >= 1.f)
            {
                BlendingState = EBlendingState::BlendedIn;
                BlendAlpha = 1.f;

                SpringArm->TargetArmLength = LastSpringArmLength;
                SpringArm->SetWorldLocation(FinalTransform.Location);
            }
            if (Cue)
            {
                const FVector Forward = Cue->GetActorForwardVector();
                AimOffset = -Forward * CueOffsetMultiplier;
                const FVector Up = Cue->GetActorUpVector();
                AimOffset += -Up * CueDownOffsetMultiplier;
            }
            UpdateCueLocation(AimOffset);
            break;

        case EBlendingState::BlendingOut:
            EasedAlpha = UKismetMathLibrary::Ease(1.f, 0.f, BlendAlpha, EEasingFunc::ExpoOut);
            // A player can move while blending, so update final location all the time.
            FinalTransform.Location = GetDefaultCameraSpringWorldLocation();
            // keep final rotation updated, as we would like to watch at the launched ball
            FinalTransform.Rotation = (ActorToLookAtWhileBlending->GetActorLocation() - GetDefaultCameraSpringWorldLocation()).ToOrientationQuat();

            CurrentTransform.Location = FMath::Lerp(StartingTransform.Location, FinalTransform.Location, EasedAlpha);
            CurrentTransform.Rotation = FMath::Lerp(StartingTransform.Rotation, FinalTransform.Rotation, EasedAlpha);

            MyOwner->GetController()->SetControlRotation(CurrentTransform.Rotation.Rotator());
            SpringArm->SetWorldLocation(CurrentTransform.Location);

            SpringArm->TargetArmLength = FMath::Lerp(LastSpringArmLength, 0.f, EasedAlpha);

            BlendAlpha -= DeltaTime * BlendingSpeed;

            if (BlendAlpha <= 0.f)
            {
                BlendingState = EBlendingState::BlendedOut;
                BlendAlpha = 0.f;

                SpringArm->TargetArmLength = 0.f;
                SpringArm->SetWorldLocation(FinalTransform.Location);
            }
            break;
    }
}

void UBilliardistAimingComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UBilliardistAimingComponent, Cue);
}

bool UBilliardistAimingComponent::Server_CreateCue_Validate(const FVector& Location, const FQuat& Rotation)
{
    return true;
}

bool UBilliardistAimingComponent::Server_MoveCue_Validate(const FVector& Location, const FQuat& Rotation)
{
    return true;
}

bool UBilliardistAimingComponent::Server_DestroyCue_Validate()
{
    return true;
}