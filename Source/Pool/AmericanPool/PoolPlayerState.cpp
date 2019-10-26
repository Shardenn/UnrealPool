// Copyright 2019 Andrei Vikarchuk.

#include "PoolPlayerState.h"
#include "../Pool.h"
#include "AmericanPool/PoolGameState.h"
#include "AmericanPool/PoolGameMode.h"
#include "Objects/BallAmerican.h"

#include "Player/Billiardist/BilliardistPawn.h" // TODO dependency invertion is violated

#include "UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"

bool APoolPlayerState::Server_ToggleReady_Validate() { return true; }
void APoolPlayerState::Server_ToggleReady_Implementation()
{
    bIsReady = !bIsReady;

    APoolGameState* GameState = Cast<APoolGameState>(UGameplayStatics::GetGameState(GetWorld()));
    if (!ensure(GameState != nullptr)) return;

    uint32 Offset = bIsReady ? 1 : -1;
    GameState->SetPlayersReadyNum(GameState->PlayersReadyNum + Offset);
}

bool APoolPlayerState::PlaceCueBall_Validate(const FVector&) { return true; }
void APoolPlayerState::PlaceCueBall_Implementation(const FVector& TablePoint) const
{
    if (!CueBallHanded)
    {
        UE_LOG(LogPool, Warning, TEXT("PlaceCueBall: CueBallHanded == nullptr"));
        return;
    }

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
    Cast<ABilliardistPawn>(GetPawn())->NotifyTurnUpdate(bMyTurn);
}

//bool APoolPlayerState::SetBallInHand_Validate(ABall* CueBall) { return true; }
void APoolPlayerState::SetBallInHand/*_Implementation*/(ABall* CueBall)
{
    CueBallHanded = CueBall;

    Cast<ABilliardistPawn>(GetPawn())->Client_NotifyBallInHand(CueBall != nullptr);
}

void APoolPlayerState::AssignBallType(FBallType Type)
{
    AssignedBallType = Type;
}

bool APoolPlayerState::HandleFrameWon_Validate() { return true; }
void APoolPlayerState::HandleFrameWon_Implementation()
{
    FramesWon++;
}

void APoolPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APoolPlayerState, bIsReady);
    DOREPLIFETIME(APoolPlayerState, bMyTurn);
    DOREPLIFETIME(APoolPlayerState, bBallInHand);
    DOREPLIFETIME(APoolPlayerState, CueBallHanded);
    DOREPLIFETIME(APoolPlayerState, AssignedBallType);
    DOREPLIFETIME(APoolPlayerState, FramesWon);
}
