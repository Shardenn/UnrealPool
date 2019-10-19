// Copyright 2019 Andrei Vikarchuk.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BilliardistReplicationComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class POOL_API UBilliardistReplicationComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UBilliardistReplicationComponent();

    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    void SetPlayerState(class APlayerState* State);
    const class APoolPlayerState* GetPlayerState() const { return PlayerState; }

    void ReadyStateToggle();
protected:
    virtual void BeginPlay() override;

    class APoolPlayerState* PlayerState = nullptr;

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_UpdatePlayerLocation(const FVector& NewLocation);
private:
    class UBilliardistMovementComponent* MovementComponent = nullptr;
};
