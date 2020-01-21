// Copyright 2019 Andrei Vikarchuk.


#include "FrameState.h"

#include "Pool.h"

#include "GameplayLogic/TurnState.h"
#include "Objects/Ball.h"

//#include "GameFramework/PlayerState.h"
#include "GameFramework/GameState.h"
#include "UnrealNetwork.h"

UFrameState::UFrameState()
{
    TurnState = NewObject<UTurnState>(this, TurnStateClass);
}

void UFrameState::Initialise(AGameState* InGameState)
{
    if (!ensure(InGameState != nullptr))
        return;
    
    this->GameState = InGameState;
    this->PlayerArray = GameState ->PlayerArray;
}

UFrameState::~UFrameState()
{
    //delete TurnState ??
}

void UFrameState::Restart()
{
    TurnState->ClearTurnStateVariables();

    //CueBall = nullptr;

    ClearFrameStateVariables();

    /*
    for (auto& Player : PlayerArray)
    {
        APoolPlayerState* PoolPlayer = Cast<APoolPlayerState>(Player);
        if (PoolPlayer)
            PoolPlayer->AssignBallType(FBallType::NotInitialized);
    }
    */
}

//bool UFrameState::Server_SwitchTurn_Validate() { return true; }
void UFrameState::Server_SwitchTurn()//_Implementation()
{
    APlayerState* FormerPlayerTurn = PlayerArray[PlayerIndexTurn];
    //FormerPlayerTurn->SetIsMyTurn(false);

    PlayerIndexTurn = (PlayerIndexTurn + 1) % PlayerArray.Num();
    UE_LOG(LogPool, Warning, TEXT("Turn is on player indexed %d"), PlayerIndexTurn);
    APlayerState* NewPlayerTurn = PlayerArray[PlayerIndexTurn];
    //NewPlayerTurn->SetIsMyTurn(true);
}

bool UFrameState::RequestIsPlayerTurn(const APlayerState* PlayerState)
{
    if (TurnState->GetMovingBalls().Num() > 0)
        return false;

    if (PlayerArray[PlayerIndexTurn] == PlayerState)
        return true;

    return false;
}

void UFrameState::ClearFrameStateVariables()
{
    //TurnState = nullptr;
    PlayerIndexTurn = 0;
    m_PlayerWithCueBall = nullptr;

    bool m_bTableOpened = true;
    bool m_bBallsRackBroken = false;
    bool m_bShouldSwitchTurn = true;

    m_ActiveBalls.Empty();
    m_DroppedBalls.Empty();
    m_PocketedBalls.Empty();
    m_BallsPlayedOutOfGame.Empty();
}

//void UFrameState::Server_RegisterBall_Validate(ABall*) { return true; }
void UFrameState::Server_RegisterBall/*_Implementation*/(ABall* Ball)
{
    m_BallsPlayedOutOfGame.AddUnique(Ball);
}
/*
void UFrameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UFrameState, PlayerIndexTurn);
}*/