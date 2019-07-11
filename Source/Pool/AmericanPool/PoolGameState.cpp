#include "PoolGameState.h"

#include "Pool.h"
#include "Objects/Ball.h"

#include "UnrealNetwork.h"
#include "GameFramework/PlayerState.h"

void APoolGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APoolGameState, PlayersReadyNum);
    DOREPLIFETIME(APoolGameState, PlayerIndexTurn); 
}

void APoolGameState::BeginPlay()
{
    Super::BeginPlay();
}

bool APoolGameState::SetPlayersReadyNum_Validate(uint32 PlayersReady) { return true; }
void APoolGameState::SetPlayersReadyNum_Implementation(uint32 PlayersReady)
{
    PlayersReadyNum = PlayersReady;
}

void APoolGameState::AddMovingBall(UPrimitiveComponent* Comp, FName BoneName)
{
    ABall* NewBall = Cast<ABall>(Comp->GetOwner());
    if (!NewBall)
        return;

    MovingBalls.AddUnique(NewBall);
}

void APoolGameState::RemoveMovingBall(UPrimitiveComponent* Comp, FName BoneName)
{
    ABall* Ball = Cast<ABall>(Comp->GetOwner());
    if (!Ball)
        return;

    if (MovingBalls.Contains(Ball))
        MovingBalls.Remove(Ball);

    if (MovingBalls.Num() == 0)
    {
        PlayerIndexTurn = (PlayerIndexTurn + 1) % PlayerArray.Num();
        UE_LOG(LogPool, Warning, TEXT("Turn is on player indexed %d"), PlayerIndexTurn);
    }
}

bool APoolGameState::RequestIsPlayerTurn(APlayerState* PlayerState)
{
    if (MovingBalls.Num() > 0)
        return false;

    if (PlayerArray[PlayerIndexTurn] == PlayerState)
        return true;

    return false;
}