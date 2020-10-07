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
    BlendingOut
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
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Setup")
    void Initialize(USpringArmComponent* InSpringArm);

    float GetHitStrength() { return HitStrength; }
    float GetMaxHitStrength() { return MaxAcceptableHitStrength; }

    void UpdateHitStrengthRatio(float Delta);

    void AdjustZoom(float Delta);

    void HandleStartedAiming(const FVector& AimedAt);
    void HandleFinishedAiming(class AActor* const ActorToLookAt);
protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    USpringArmComponent* SpringArm = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector DefaultSpringArmLocation = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hit strength")
    float HitStrength = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit strength")
    float HitStrengthadjustmentSpeed = 0.05f;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Hit strength")
    float HitStrengthRatio = 0.f;

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
    FVector GetDefaultCameraSpringWorldLocation() const { return DefaultSpringArmLocation + GetOwner()->GetActorLocation(); }
    float LastSpringArmLength = ZoomSpringArmLengthMax * 0.2f;
};
