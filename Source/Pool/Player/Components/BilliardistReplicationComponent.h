// Copyright 2019 Andrei Vikarchuk.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Misc/Optional.h"
#include "BilliardistReplicationComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class POOL_API UBilliardistReplicationComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UBilliardistReplicationComponent();

    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    void SetPlayerState(class APlayerState* State);
    class APoolPlayerState* GetPlayerState() const { return PlayerStateBill; }

    void ReadyStateToggle();

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_PerformBallHit(class ABall* Ball, const FVector& Velocity, const FVector& AtLocation = FVector(0));
protected:
    virtual void BeginPlay() override;

    UPROPERTY(Replicated)
    class APoolPlayerState* PlayerStateBill = nullptr;

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_UpdatePlayerLocation(const FVector& NewLocation);

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_SetPlayerState(class APlayerState* State);
private:
    class UBilliardistMovementComponent* MovementComponent = nullptr;
};
