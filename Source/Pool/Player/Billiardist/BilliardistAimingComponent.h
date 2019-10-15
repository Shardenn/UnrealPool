// Copyright 2019 Andrei Vikarchuk.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BilliardistAimingComponent.generated.h"

class USpringArmComponent;

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

    void HandleStartedAiming(const FVector& AimedAt);
    void HandleFinishedAiming();
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
};
