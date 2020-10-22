// Copyright 2019 Andrei Vikarchuk.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BilliardistAimingComponent.generated.h"

class USpringArmComponent;

enum class EBlendingState : uint8
{
    None,
    BlendingIn,
    BlendingOut,
    BlendedIn,
    BlendedOut
};

struct InterpolationData
{
    FVector Location;
    FQuat Rotation;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class POOL_API UBilliardistAimingComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UBilliardistAimingComponent();
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Setup")
    void Initialize(USpringArmComponent* InSpringArm);

    float GetHitStrength() { return HitStrength; }
    float GetMaxHitStrength() { return MaxAcceptableHitStrength; }

    void UpdateHitStrengthRatio(float Delta);

    void AdjustZoom(float Delta);

    void HandleStartedAiming(const FVector& AimedAt);
    void HandleFinishedAiming(class AActor* const ActorToLookAt);

    void SpinHorizontalUpdate(const float Value);
    void SpinVerticalUpdate(const float Value);
protected:
    virtual void BeginPlay() override;

    // Needed for cue placement
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cue placement")
    float BallRadius{ 2.6f };
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cue placement")
    float CuePlacementThreshold{ 0.5f };

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cue placement | Camera adjustment")
    float CueRotationOffsetFromControlRotation{ 20.f };
    // As we manually rotate cue a bit down (in order not to collide with camera)
    // we should also move it a bit down, that way it looks better
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cue placement | Camera adjustment")
    float CueDownOffsetMultiplier{ 1.f };
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cue placement | Swinging")
    float CuePositionOffsetStepWhileAiming{ 1.f };
    // Offset during the swinging
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cue placement | Swinging")
    float MaxCueOffsetMultiplier{ 25.f };

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cue placement | Overlap compensation")
    float OverlapCompensationRotationStep{ 5.f };
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cue placement | Overlap compensation")
    float OverlapCompensationRotationMax{ 5.f };

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cue placement | Spinning")
    float CueSpinOffsetMax{ 1.5f };
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Cue placement")
    class ACue* Cue{ nullptr };

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cue placement")
    TSubclassOf<class ACue> CueClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    USpringArmComponent* SpringArm = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector DefaultSpringArmLocation = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hit strength")
    float HitStrength = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit strength")
    float HitStrengthadjustmentSpeed = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit strength")
    float MaxAcceptableHitStrength = 1000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera controls")
    float ZoomAdjustementSpeed = 1.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera controls")
    float ZoomSpringArmLengthMax = 200.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera controls")
    float ZoomSpringArmLengthMin = 15.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera controls")
    float BlendingSpeed = 2.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera controls")
    float EaseExponent = 0.5f;

#pragma region Interpolation
    EBlendingState BlendingState = EBlendingState::None;
    float BlendAlpha = 0.0;
    InterpolationData CurrentTransform;
    InterpolationData StartingTransform, FinalTransform;
    // Follow an actor while blending by watching on it constantly
    class AActor* ActorToLookAtWhileBlending = nullptr;
#pragma endregion

private:
    float HitStrengthRatio{ 0.f };
    // Offset from ball to the player.
    // Used for cue swinging while strength adjustment
    float CueOffsetMultiplier{ 0.f };

    FRotator CurrentCueOverlapOffset{ 0.f, 0.f, 0.f };

    float CueSpinOffsetX{ 0.f };
    float CueSpinOffsetY{ 0.f };
    
    FVector GetDefaultCameraSpringWorldLocation() const
    {
        return DefaultSpringArmLocation + GetOwner()->GetActorLocation();
    }

    float LastSpringArmLength = ZoomSpringArmLengthMax * 0.2f;

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_CreateCue(const FVector& Location, const FQuat& Rotation);
    UFUNCTION(Server, Reliable, WithValidation)
    void Server_DestroyCue();
    UFUNCTION(Server, Reliable, WithValidation)
    void Server_MoveCue(const FVector& Location, const FQuat& Rotation);

    class APawn* MyOwner{ nullptr };

    void GetCueLocationAndRotation(FVector& Location, FQuat& Rotation);

    void UpdateCueLocation(const FVector& AimOffset = FVector(0, 0, 0), const FQuat& RotationOffset = FQuat::Identity);

    bool bInControlOfPawn{ false };
};
