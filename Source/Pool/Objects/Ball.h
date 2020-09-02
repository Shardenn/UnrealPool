// Copyright 2019 Andrei Vikarchuk.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Ball.generated.h"

USTRUCT()
struct FSmoothPhysicsState
{
    GENERATED_BODY()

public:
    UPROPERTY()
    uint64 TimeStamp;

    UPROPERTY()
    FVector Location;

    UPROPERTY()
    FVector Velocity;

    UPROPERTY()
    FQuat Rotation;

    FSmoothPhysicsState()
    {
        TimeStamp = 0;
        Location = FVector::ZeroVector;
        Velocity = FVector::ZeroVector;
        Rotation = FQuat::Identity;
    }

};

UENUM(BlueprintType)
enum class FInterpolationType : uint8
{
    Linear UMETA(DisplayName = "Linear"),
    Cubic  UMETA(DisplayName = "Cubic")
};

struct FHermiteCubicSpline
{
    FVector StartLocation, StartDerivative, TargetLocation, TargetDerivative;

    FVector InterpolateLocationLinear(const float LerpRatio) const
    {
        return FMath::Lerp(StartLocation, TargetLocation, LerpRatio);
    }

    FVector InterpolateLocationCubic(const float LerpRatio) const
    {
        return FMath::CubicInterp(StartLocation, StartDerivative, TargetLocation, TargetDerivative, LerpRatio);
    }

    FVector InterpolateDerivative(const float LerpRatio) const
    {
        return FMath::CubicInterpDerivative(StartLocation, StartDerivative, TargetLocation, TargetDerivative, LerpRatio);
    }
};

UCLASS()
class POOL_API ABall : public AActor
{
    GENERATED_BODY()

public:
    ABall();

    virtual void RemoveBallFromGame();
    virtual void ReturnBallIntoGame();
protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FVector SpawnLocation = FVector(0);

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FInterpolationType InterpolationType{ FInterpolationType::Cubic };

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    UStaticMeshComponent* SphereMesh;

    UPROPERTY(ReplicatedUsing = OnRep_SmoothPhysicsState)
    FSmoothPhysicsState ServerPhysicsState;
    
    UFUNCTION()
    void OnRep_SmoothPhysicsState();
    
protected:
    void ClientTick(float DeltaTime);

    FHermiteCubicSpline CreateSpline();
    void InterpolateLocation(const FHermiteCubicSpline& Spline, const float Ratio);
    void InterpolateRotation(const float Ratio);
    void InterpolateVelocity(const FHermiteCubicSpline& Spline, const float Ratio);

    // * 100 because we are measuring in m/sec, while Unreal's default unit is cm
    float VelocityToDerivative() { return ClientTimeBetweenLastUpdates; }

    float ClientTimeSinceUpdate{ 0.0 };
    float ClientTimeBetweenLastUpdates{ 0.0 };

    FTransform ClientStartTransform;
    FVector ClientStartVelocity{ FVector(0) };
    FVector ClientLastKnownVelocity{ FVector(0) };

    FVector ClientStartDerivative{ FVector(0) };
    FVector ClientLastKnownDerivative{ FVector(0) };
};
