// Fill out your copyright notice in the Description page of Project Settings.

#include "../Pool.h"

#include "PoolPlayerState.h"
#include "AmericanPool/PoolGameState.h"
#include "Objects/BallAmerican.h"

#include "UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"

void APoolPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APoolPlayerState, bIsReady);
    DOREPLIFETIME(APoolPlayerState, bMyTurn);
    DOREPLIFETIME(APoolPlayerState, bBallInHand);
}

bool APoolPlayerState::ToggleReady_Validate() { return true; }
void APoolPlayerState::ToggleReady_Implementation()
{
    bIsReady = !bIsReady;

    APoolGameState* GameState = Cast<APoolGameState>(UGameplayStatics::GetGameState(GetWorld()));
    if (!ensure(GameState != nullptr)) return;

    uint32 Offset = bIsReady ? 1 : -1;
    GameState->SetPlayersReadyNum(GameState->PlayersReadyNum + Offset);
}

bool APoolPlayerState::PlaceCueBall_Validate(const FVector&) { return true; }
void APoolPlayerState::PlaceCueBall_Implementation(const FVector& TablePoint)
{
    if (!CueBallHanded)
        return;
    
    UWorld* World = GetWorld();
    if (!World) return;

    float BallRadius = CueBallHanded->GetRootComponent()->Bounds.SphereRadius;
    
    CueBallHanded->SetActorLocation(TablePoint + FVector(0, 0, BallRadius));
    Cast<UPrimitiveComponent>(CueBallHanded->GetRootComponent())->SetSimulatePhysics(true);

    APoolGameState* State = Cast<APoolGameState>(UGameplayStatics::GetGameState(World));
    if (!ensure(State != nullptr)) return;

    State->TakeBallFromHand();
}

//bool APoolPlayerState::SetIsMyTurn_Validate(bool bInIsMyTurn) { return true; }
void APoolPlayerState::SetIsMyTurn(bool bInIsMyTurn)
{
    bMyTurn = bInIsMyTurn;
}

void APoolPlayerState::SetBallInHand(ABall* CueBall)
{
    this->CueBallHanded = CueBall;
}