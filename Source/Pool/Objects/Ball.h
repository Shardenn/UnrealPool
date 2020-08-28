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
    FVector Position;

    UPROPERTY()
    FVector Velocity;

    UPROPERTY()
    FQuat Rotation;

    FSmoothPhysicsState()
    {
        TimeStamp = 0;
        Position = FVector::ZeroVector;
        Velocity = FVector::ZeroVector;
        Rotation = FQuat::Identity;
    }

};

struct FHermiteCubicSpline
{
    FVector StartLocation, StartDerivative, TargetLocation, TargetDerivative;

    FVector InterpolateLocation(const float LerpRatio) const
    {
        //return FMath::CubicInterp(StartLocation, StartDerivative, TargetLocation, TargetDerivative, LerpRatio);
        return FMath::LerpStable(StartLocation, TargetLocation, LerpRatio);
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

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    UStaticMeshComponent* SphereMesh;

    UPROPERTY(ReplicatedUsing = OnRep_SmoothPhysicsState)
    FSmoothPhysicsState ServerPhysicsState;
    
    UFUNCTION()
    void OnRep_SmoothPhysicsState();
    
private:
    void ClientTick(float DeltaTime);

    FHermiteCubicSpline CreateSpline();
    void InterpolateLocation(const FHermiteCubicSpline& Spline, const float Ratio);
    void InterpolateRotation(const float Ratio);
    void InterpolateVelocity(const FHermiteCubicSpline& Spline, const float Ratio);

    // * 100 because we are measuring in m/sec, while Unreal's default unit is cm
    float VelocityToDerivative() { return ClientTimeBetweenLastUpdates * 100; }

    float ClientTimeSinceUpdate{ 0.0 };
    float ClientTimeBetweenLastUpdates{ 0.0 };

    FTransform ClientStartTransform;
    FVector ClientStartVelocity;

    static const uint8 PROXY_STATE_ARRAY_SIZE_MAX{ 32 };
    uint8 ProxyStateCountCurrent{ 0 };
    uint8 ProxyStateLatestIndex{ 0 };
    TCircularBuffer<FSmoothPhysicsState> ProxyStates{ PROXY_STATE_ARRAY_SIZE_MAX };
};
