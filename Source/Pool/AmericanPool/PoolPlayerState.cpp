// Fill out your copyright notice in the Description page of Project Settings.

#include "PoolPlayerState.h"
#include "../Pool.h"
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
    DOREPLIFETIME(APoolPlayerState, CueBallHanded);
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
    
    // If we first SetLocation and then SimulatePhys(true), then SetLocation not working
    Cast<UPrimitiveComponent>(CueBallHanded->GetRootComponent())->SetSimulatePhysics(true);
    CueBallHanded->SetActorLocation(TablePoint + FVector(0, 0, BallRadius + 1));

    APoolGameState* State = Cast<APoolGameState>(UGameplayStatics::GetGameState(World));
    if (!ensure(State != nullptr)) return;

    State->TakeBallFromHand();
}

//bool APoolPlayerState::SetIsMyTurn_Validate(bool bInIsMyTurn) { return true; }
void APoolPlayerState::SetIsMyTurn(bool bInIsMyTurn)
{
    bMyTurn = bInIsMyTurn;
}

//bool APoolPlayerState::SetBallInHand_Validate(ABall* CueBall) { return true; }
void APoolPlayerState::SetBallInHand/*_Implementation*/(ABall* CueBall)
{
    CueBallHanded = CueBall;

    FString BallName = CueBall ? *CueBall->GetName() : FString("NULL");
}