#include "PoolGameState.h"

#include "Pool.h"
#include "Objects/Ball.h"

#include "UnrealNetwork.h"
#include "PoolPlayerState.h"

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

void APoolGameState::OnRep_UpdatePlayerStateTurn()
{
    uint32 FormerIndex = (PlayerIndexTurn - 1);
    if (FormerIndex < 0)
        FormerIndex += PlayerArray.Num();

    Cast<APoolPlayerState>(PlayerArray[FormerIndex])->SetIsMyTurn(false);
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

    if (MovingBalls.Num() > 0)
    {
        APoolPlayerState* PlayerTurn = Cast<APoolPlayerState>(PlayerArray[PlayerIndexTurn]);
        PlayerTurn->SetIsMyTurn(false);
    }
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
        SwitchTurn();
    }
}

bool APoolGameState::SwitchTurn_Validate() { return true; }
void APoolGameState::SwitchTurn_Implementation()
{
    APoolPlayerState* FormerPlayerTurn = Cast<APoolPlayerState>(PlayerArray[PlayerIndexTurn]);
    FormerPlayerTurn->SetIsMyTurn(false);

    PlayerIndexTurn = (PlayerIndexTurn + 1) % PlayerArray.Num();
    UE_LOG(LogPool, Warning, TEXT("Turn is on player indexed %d"), PlayerIndexTurn);
    APoolPlayerState* NewPlayerTurn = Cast<APoolPlayerState>(PlayerArray[PlayerIndexTurn]);
    NewPlayerTurn->SetIsMyTurn(true);
}

bool APoolGameState::RequestIsPlayerTurn(APlayerState* PlayerState)
{
    if (MovingBalls.Num() > 0)
        return false;

    if (PlayerArray[PlayerIndexTurn] == PlayerState)
        return true;

    return false;
}