// Copyright 2019 Andrei Vikarchuk.
#include "BilliardistReplicationComponent.h"

#include "AmericanPool/PoolPlayerState.h"
#include "BilliardistMovementComponent.h"

UBilliardistReplicationComponent::UBilliardistReplicationComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    SetIsReplicated(true);
}

void UBilliardistReplicationComponent::BeginPlay()
{
    Super::BeginPlay();
    
    MovementComponent = GetOwner()->FindComponentByClass<UBilliardistMovementComponent>();
}

void UBilliardistReplicationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!MovementComponent) return;

    if (GetOwnerRole() == ROLE_AutonomousProxy ||
        GetOwner()->GetRemoteRole() == ROLE_SimulatedProxy)
    {
        FVector LastPlayerPos = MovementComponent->GetLastPlayerLocation();
        Server_UpdatePlayerLocation(LastPlayerPos);
    }
}

void UBilliardistReplicationComponent::SetPlayerState(APlayerState* InState)
{
    auto State = Cast<APoolPlayerState>(InState);
    PlayerState = State;

    if (!PlayerState)
        UE_LOG(LogPlayerManagement, Warning, TEXT("PlayerState in BilliardistReplicationComponent (%s) is NULL"),
            *GetName());
}

void UBilliardistReplicationComponent::ReadyStateToggle()
{
    if (!PlayerState) return;
    PlayerState->ToggleReady();
}

bool UBilliardistReplicationComponent::Server_UpdatePlayerLocation_Validate(const FVector&) { return true; }
void UBilliardistReplicationComponent::Server_UpdatePlayerLocation_Implementation(const FVector& NewLocation)
{
    GetOwner()->SetActorLocation(NewLocation);
}