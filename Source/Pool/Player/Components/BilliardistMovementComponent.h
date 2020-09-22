// Copyright 2019 Andrei Vikarchuk.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BilliardistMovementComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class POOL_API UBilliardistMovementComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UBilliardistMovementComponent();

    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    void SetForwardIntent(float InForwardIntent);
    void SetRightIntent(float InRightIntent);

    void SetSpline(class USplineComponent* InSpline) { Spline = InSpline; }

    FVector GetLastPlayerLocation() { return LastPlayerLocation; }

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WalkSpeed{ 200.f };

    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite)
    class USplineComponent* Spline = nullptr;
private:
    float ForwardIntent = 0;
    float RightIntent = 0;
    FVector WalkIntent = FVector::ZeroVector;

    // calculates new spline point based on WalkIntent
    void CalculateNewSplinePoint(float DeltaTime);

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_GetSpline();

    float SplineDistance = 0;
    FVector LastPlayerLocation = FVector::ZeroVector;
};
