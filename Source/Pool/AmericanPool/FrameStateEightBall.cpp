// Copyright 2019 Andrei Vikarchuk.


#include "FrameStateEightBall.h"

#include "Pool.h"

#include "GameplayLogic/TurnState.h"
#include "Objects/BallAmerican.h"
#include "AmericanPool/PoolPlayerState.h"
#include "AmericanPool/PoolGameMode.h"
#include "AmericanPool/PoolGameState.h"

void UFrameStateEightBall::Server_HandleTurnEnd()
{
    if (!TurnState) return;

    auto BallsPocketed = TurnState->GetPocketedBalls();

    for (auto& InBall : BallsPocketed)
    {
        auto Ball = Cast<ABallAmerican>(InBall);
        if (!Ball)
            continue;

        FBallType Type = Ball->GetType();

        if (Type == FBallType::Black)
        {
            HandleBlackBallOutOfPlay();
            return;
        }
        else if (Type == FBallType::Cue)
        {
            AssignFoul();
        }

        // TODO handle named shot

        if (!m_bTableOpened)
        {
            APoolPlayerState* Player = Cast<APoolPlayerState>(PlayerArray[PlayerIndexTurn]);
            FBallType CurrentPlayerBallType = Player->GetAssignedBallType();

            if (Ball->GetType() == CurrentPlayerBallType)
                m_bShouldSwitchTurn = false;
        }

        if (Type != FBallType::Cue)
            RegisterBall(Ball);
    }

    auto DroppedBalls = TurnState->GetDroppedBalls();

    for (auto& InBall : DroppedBalls)
    {
        auto Ball = Cast<ABallAmerican>(InBall);
        FBallType Type = Ball->GetType();

        if (Type == FBallType::Black)
        {
            HandleBlackBallOutOfPlay();
            return;
        }
        else if (Type == FBallType::Cue)
        {
            UE_LOG(LogPool, Warning, TEXT("Cue ball is assigned: %s"), *CueBall->GetName());
            AssignFoul();
        }

        if (Type != FBallType::Cue)
            RegisterBall(Ball);
    }

    auto PocketedBalls = TurnState->GetPocketedBalls();
    if (m_bTableOpened &&
        PocketedBalls.Num() > 0)
    {
        m_bShouldSwitchTurn = false;
    }

    auto BallsHittedByTheCue = TurnState->GetBallsHittedByTheCue();
    if (BallsHittedByTheCue.Num() == 0)
    {
        AssignFoul();
    }

    // assign balls type if not done yet
    if (PocketedBalls.Num() > 0 &&
        m_bBallsRackBroken &&
        m_bTableOpened &&
        !m_bPlayerFouled)
    {
        FBallType CurrentAssignedType = FBallType::NotInitialized,
            OtherAssignedType = FBallType::NotInitialized;
        for (auto& InBall : PocketedBalls)
        {
            auto Ball = Cast<ABallAmerican>(InBall);
            if (Ball->GetType() == FBallType::Solid)
            {
                CurrentAssignedType = FBallType::Solid;
                OtherAssignedType = FBallType::Stripe;
                break;
            }
            else if (Ball->GetType() == FBallType::Stripe)
            {
                CurrentAssignedType = FBallType::Stripe;
                OtherAssignedType = FBallType::Solid;
                break;
            }
        }

        APoolPlayerState* Player = Cast<APoolPlayerState>(PlayerArray[PlayerIndexTurn]);
        Player->AssignBallType(CurrentAssignedType);

        // if we are playing standalone, then do not reassign another type to the player
        if (PlayerArray.Num() > 1)
        {
            auto OtherPlayerIndex = (PlayerIndexTurn + 1) % PlayerArray.Num();
            Player = Cast<APoolPlayerState>(PlayerArray[OtherPlayerIndex]);
            Player->AssignBallType(OtherAssignedType);
        }

        // now the types are assigned
        m_bTableOpened = false;
    }

    if (BallsHittedByTheCue.Num() > 0)
    {
        if (!m_bBallsRackBroken)
            m_bBallsRackBroken = true;
    }

    if (m_bShouldSwitchTurn || m_bPlayerFouled)
        Server_SwitchTurn();
    else
    {
        // TODO move to other method
        APoolPlayerState* Player = Cast<APoolPlayerState>(PlayerArray[PlayerIndexTurn]);
        if (Player)
            Player->SetIsMyTurn(true);
    }

    if (m_bPlayerFouled)
    {
        if (!CueBall)
        {
            UE_LOG(LogPool, Warning, TEXT("Tried to give ball in hand, but CueBall is NULL"));
        }
        else
        {
            //Server_GiveBallInHand();
        }
    }

    TurnState->ClearTurnStateVariables();
}

void UFrameStateEightBall::HandleBlackBallOutOfPlay()
{
    APoolGameMode* GM = Cast<APoolGameMode>(GameState->AuthorityGameMode);
    if (!ensure(GM != nullptr)) return;

    if (!m_bBallsRackBroken)
    {
        GM->RestartFrame();
    }
    else
    {
        uint8 NewFramesWon = 0;
        uint8 WonPlayerIndex = 0;
        APoolPlayerState* WonPoolPlayer = nullptr;

        bool bCurrentPlayerWon = DecideWinCondition();
        if (bCurrentPlayerWon)
        {
            WonPlayerIndex = PlayerIndexTurn;
        }
        else
        {
            WonPlayerIndex = (PlayerIndexTurn + 1) % PlayerArray.Num();
        }

        WonPoolPlayer = Cast<APoolPlayerState>(PlayerArray[WonPlayerIndex]);
        WonPoolPlayer->HandleFrameWon();
        NewFramesWon = WonPoolPlayer->GetFramesWon();

        if (NewFramesWon >= GM->RequiredFramesToWin)
            GM->EndMatch();
        else
            GM->RestartFrame();
    }
}

bool UFrameStateEightBall::DecideWinCondition()
{
    // when 8 ball is scored and the rack was already broken,
    // there are two options: current player won or lose.

    // If balls of his types are all scored AND
    // on the 8ball pocketing he did not pocket any other ball,
    // then he won.
    // Otherwise - other player won.

    auto GM = Cast<APoolGameMode>(GameState->AuthorityGameMode);
    // we return true cause in case we have some error
    // with getting GM and we return false, then the players
    // are stuck with the situation
    if (!ensure(GM != nullptr)) return true;

    APoolPlayerState* PoolPlayer = Cast<APoolPlayerState>(PlayerArray[PlayerIndexTurn]);
    if (!ensure(PoolPlayer != nullptr)) return true;

    FBallType PlayersType = PoolPlayer->GetAssignedBallType();
    int32 BallsOfTypePlayedOut = 0;
    for (auto& InPlayedOutBall : m_BallsPlayedOutOfGame)
    {
        auto PlayedOutBall = Cast<ABallAmerican>(InPlayedOutBall);
        if (PlayedOutBall->GetType() == PlayersType)
            ++BallsOfTypePlayedOut;
    }

    // if there are no balls of our type on the table
    if (BallsOfTypePlayedOut < GM->GetRequiredBallsToPocket())
        return false;
    // if we pocketed smth else otherwise than 8ball, we lose
    if (TurnState->GetPocketedBalls().Num() > 1)
        return false;

    return true;
}