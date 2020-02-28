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
    FRotator Rotation;

    FSmoothPhysicsState()
    {
        TimeStamp = 0;
        Position = FVector::ZeroVector;
        Velocity = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
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
    static const uint8 PROXY_STATE_ARRAY_SIZE_MAX{ 20 };
    uint8 ProxyStateCountCurrent{ 0 };
    // The most relevant state is stored in ProxyStates[0]
    TArray<FSmoothPhysicsState> ProxyStates;

    void ClientSimulateFreeMovingBall();
};
