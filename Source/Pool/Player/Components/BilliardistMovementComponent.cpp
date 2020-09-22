// Copyright 2019 Andrei Vikarchuk.

#include "BilliardistMovementComponent.h"
#include "Pool.h"

#include "GameplayLogic/PoolGameMode.h"

#include "Components/SplineComponent.h"
#include "Kismet/GameplayStatics.h"

UBilliardistMovementComponent::UBilliardistMovementComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    SetIsReplicatedByDefault(true);
}

void UBilliardistMovementComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UBilliardistMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!Spline)
    {
        if (GetOwnerRole() == ROLE_AutonomousProxy ||
            GetOwner()->GetRemoteRole() == ROLE_SimulatedProxy)
            Server_GetSpline();
        return;
    }

    if (WalkIntent != FVector::ZeroVector)
    {
        CalculateNewSplinePoint(DeltaTime);
        LastPlayerLocation = Spline->GetLocationAtDistanceAlongSpline(SplineDistance,
            ESplineCoordinateSpace::World);
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

bool UBilliardistMovementComponent::Server_GetSpline_Validate() { return true; }
void UBilliardistMovementComponent::Server_GetSpline_Implementation()
{
    auto GM = Cast<APoolGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
    if (!GM) return;

    auto SplineComp = GM->GetSpline();
    SetSpline(SplineComp);
}

void UBilliardistMovementComponent::SetForwardIntent(float InForwardIntent)
{
    auto OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn) return;

    auto Rot = OwnerPawn->GetController()->GetControlRotation();
    FVector ForwardIntentVec = Rot.Vector() * InForwardIntent;
    WalkIntent += ForwardIntentVec;
}

void UBilliardistMovementComponent::SetRightIntent(float InRightIntent)
{
    auto OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn) return;

    auto Rot = OwnerPawn->GetController()->GetControlRotation();
    FVector RightVector = FVector::CrossProduct(Rot.Vector(), FVector(0, 0, -1));
    FVector RightIntentVec = RightVector * InRightIntent;
    WalkIntent += RightIntentVec;
}

void UBilliardistMovementComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UBilliardistMovementComponent, Spline);
}