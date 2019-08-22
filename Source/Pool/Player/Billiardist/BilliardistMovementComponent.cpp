// Copyright 2019 Andrei Vikarchuk.

#include "BilliardistMovementComponent.h"

#include "Components/SplineComponent.h"

UBilliardistMovementComponent::UBilliardistMovementComponent()
{
    PrimaryComponentTick.bCanEverTick = true;

}

void UBilliardistMovementComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UBilliardistMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!Spline) return;

    if (WalkIntent != FVector::ZeroVector)
    {
        CalculateNewSplinePoint(DeltaTime);
        LastPlayerLocation = Spline->GetLocationAtDistanceAlongSpline(SplineDistance,
            ESplineCoordinateSpace::World);
        UE_LOG(LogTemp, Log, TEXT("SplineDist: %f, Location: %s"), SplineDistance, *LastPlayerLocation.ToString());
        GetOwner()->SetActorLocation(LastPlayerLocation);

        WalkIntent = FVector::ZeroVector;
    }
}

void UBilliardistMovementComponent::CalculateNewSplinePoint(float DeltaTime)
{
    auto SplineTangent = Spline->GetDirectionAtDistanceAlongSpline(SplineDistance, ESplineCoordinateSpace::World);
    float cosin = FVector::DotProduct(SplineTangent, WalkIntent) /
        (SplineTangent.Size() * WalkIntent.Size()); // cos between spline tangent and move direction without spline
    SplineDistance += cosin * DeltaTime * WalkSpeed;

    if (SplineDistance >= Spline->GetSplineLength())
        SplineDistance -= Spline->GetSplineLength();
    else if (SplineDistance < 0)
        SplineDistance += Spline->GetSplineLength();
}

void UBilliardistMovementComponent::SetForwardIntent(float InForwardIntent)
{
    FVector ForwardIntentVec = GetOwner()->GetActorForwardVector() * InForwardIntent;
    WalkIntent += ForwardIntentVec;
}

void UBilliardistMovementComponent::SetRightIntent(float InRightIntent)
{
    FVector RightIntentVec = GetOwner()->GetActorRightVector() * InRightIntent;
    WalkIntent += RightIntentVec;
}

