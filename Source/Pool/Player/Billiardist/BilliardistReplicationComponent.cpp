// Copyright 2019 Andrei Vikarchuk.
#include "BilliardistReplicationComponent.h"

#include "Pool.h"
#include "GameplayLogic/PoolGameState.h"
#include "GameplayLogic/PoolPlayerState.h"
#include "Objects/Ball.h" // TODO not needed much, but no better workaround
#include "BilliardistMovementComponent.h"

#include "Kismet/GameplayStatics.h"

UBilliardistReplicationComponent::UBilliardistReplicationComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    SetIsReplicatedByDefault(true);
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
    if (GetOwnerRole() == ROLE_Authority)
    {
        UE_LOG(LogPool, Warning, TEXT("AUTH: %s is set with PlayerState"), *GetName());
        if (InState)
            UE_LOG(LogPool, Warning, TEXT("%s"), *InState->GetName());
    }
    else
    {
        UE_LOG(LogPool, Warning, TEXT("REMOTE: %s is set with PlayerState"), *GetName());
        if (InState)
            UE_LOG(LogPool, Warning, TEXT("%s"), *InState->GetName());
    }
    auto State = Cast<APoolPlayerState>(InState);
    PlayerStateBill = State;

    if (!PlayerStateBill)
        UE_LOG(LogPool, Warning, TEXT("PlayerState in BilliardistReplicationComponent (%s) is NULL"),
            *GetName());
    //Server_SetPlayerState(State);
}

bool UBilliardistReplicationComponent::Server_SetPlayerState_Validate(APlayerState*) { return true; }
void UBilliardistReplicationComponent::Server_SetPlayerState_Implementation(APlayerState* InState)
{
    auto State = Cast<APoolPlayerState>(InState);
    PlayerStateBill = State;

    if (!PlayerStateBill)
        UE_LOG(LogPlayerManagement, Warning, TEXT("PlayerState in BilliardistReplicationComponent (%s) is NULL"),
            *GetName());
}

void UBilliardistReplicationComponent::ReadyStateToggle()
{
    if (!PlayerStateBill) return;
    PlayerStateBill->Server_ToggleReady();
}

bool UBilliardistReplicationComponent::Server_PerformBallHit_Validate(ABall* Ball, const FVector& Velocity) { return true; }
void UBilliardistReplicationComponent::Server_PerformBallHit_Implementation(ABall* Ball, const FVector& Velocity)
{
    UWorld* World = GetWorld();
    APoolGameState* State = Cast<APoolGameState>(UGameplayStatics::GetGameState(World));
    if (!ensure(State != nullptr)) return;

    State->Server_StartWatchingBallsMovement();

    if (!ensure(Ball != nullptr)) return;
    Cast<UStaticMeshComponent>(Ball->GetRootComponent())->AddImpulse(Velocity, NAME_None, false);
}

bool UBilliardistReplicationComponent::Server_UpdatePlayerLocation_Validate(const FVector&) { return true; }
void UBilliardistReplicationComponent::Server_UpdatePlayerLocation_Implementation(const FVector& NewLocation)
{
    GetOwner()->SetActorLocation(NewLocation);
}

void UBilliardistReplicationComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UBilliardistReplicationComponent, PlayerStateBill);
}