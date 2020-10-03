// Copyright 2019 Andrei Vikarchuk.

#include "EightBallGameState.h"
#include "Pool.h"

#include "GameplayLogic/BallsManager.h"

#include "GameplayLogic/PoolGameMode.h"
#include "GameplayLogic/Interfaces/PlayerWithHandableBall.h"
#include "EightBallGameMode.h"
#include "AmericanPool/EightBallPlayerState.h"
#include "Objects/BallAmerican.h"

#include "Objects/Table/Components/PocketArea.h"

#include "EngineUtils.h" // TObjectIterator

void AEightBallGameState::GiveBallInHand(const TScriptInterface<IPlayerWithHandableBall>& Player, ABall* const Ball)
{
    Server_GiveBallInHand(Player, Ball);
}

void AEightBallGameState::Server_GiveBallInHand_Implementation(const TScriptInterface<IPlayerWithHandableBall>& Player, ABall* Ball)
{
    GiveBallInHand_Internal(Player, Ball);
}

bool AEightBallGameState::Server_GiveBallInHand_Validate(const TScriptInterface<IPlayerWithHandableBall>& Player, ABall* Ball)
{
    return true;
}

void AEightBallGameState::GiveBallInHand_Internal(const TScriptInterface<IPlayerWithHandableBall>& Player, ABall* const Ball)
{
    if (!CueBall && !FindAndInitializeCueBall())
    {
        UE_LOG(LogPool, Warning, TEXT("GiveBallInHand: CueBall==nullptr. failed to find cue ball as well"));
        return;
    }

    auto BallPrimComp = Cast<UStaticMeshComponent>(CueBall->GetRootComponent());
    BallPrimComp->SetSimulatePhysics(false);
    CueBall->SetActorLocation(FVector(0, 0, 2000));

    check(Player);
    TakeBallFromHand(PlayerWithCueBall, CueBall);

    Player->SetBallInHand(CueBall);
    PlayerWithCueBall = Player;

    UE_LOG(LogPool, Warning, TEXT("gave ball to the player %s"), *Player.GetObject()->GetName());
}

void AEightBallGameState::TakeBallFromHand(const TScriptInterface<IPlayerWithHandableBall>& Player, ABall* Ball)
{
    Server_TakeBallFromHand(Player, Ball);
}

void AEightBallGameState::Server_TakeBallFromHand_Implementation(const TScriptInterface<IPlayerWithHandableBall>& Player, ABall* Ball)
{
    TakeBallFromHand_Internal(Player, Ball);
}

bool AEightBallGameState::Server_TakeBallFromHand_Validate(const TScriptInterface<IPlayerWithHandableBall>& Player, ABall* Ball)
{
    return true;
}

void AEightBallGameState::TakeBallFromHand_Internal(const TScriptInterface<IPlayerWithHandableBall>& Player, ABall* const Ball)
{
    if (Player)
    {
        Player->SetBallInHand(nullptr);
        PlayerWithCueBall = nullptr;
    }
}

class ABall* AEightBallGameState::GetCueBall()
{
    if (!CueBall && !FindAndInitializeCueBall())
        return nullptr;
    return CueBall;
}

void AEightBallGameState::RegisterNamedShot(UPocketArea* SelectedPocket, ABall* SelectedBall)
{
    RegisterNamedShot_Internal(SelectedPocket, SelectedBall);
}

void AEightBallGameState::RegisterNamedShot_Internal(UPocketArea* SelectedPocket, ABall* SelectedBall)
{
    UPocketArea* PreviousPocket = RegisteredNamedShot.SelectedPocket;
    if (PreviousPocket)
    {
        PreviousPocket->OnDeselected();
        PreviousPocket->OnStopHoverOver();
    }

    RegisteredNamedShot = FNamedShot(SelectedPocket, SelectedBall);
    Multicast_BroadcastNamedShotRegistered(RegisteredNamedShot);
}

void AEightBallGameState::RegisterPredictedShot(UPocketArea* SelectedPocket, ABall* SelectedBall)
{
    RegisterPredictedShot_Internal(SelectedPocket, SelectedBall);
}

void AEightBallGameState::RegisterPredictedShot_Internal(UPocketArea* SelectedPocket, ABall* SelectedBall)
{
    PredictedNamedShot = FNamedShot(SelectedPocket, SelectedBall);
    Multicast_BoradcastPredictedShotRegistered(PredictedNamedShot);
}

void AEightBallGameState::ClearTurnStateVariables()
{
    RegisteredNamedShot = FNamedShot();
    RegisterNamedShot_Internal(RegisteredNamedShot.SelectedPocket, RegisteredNamedShot.SelectedBall);
    RegisterPredictedShot_Internal(RegisteredNamedShot.SelectedPocket, RegisteredNamedShot.SelectedBall);
    Super::ClearTurnStateVariables();
}

void AEightBallGameState::OnFrameRestarted()
{
    Super::OnFrameRestarted();
    CueBall = nullptr;

    for (auto& Player : PlayerArray)
    {
        auto PoolPlayer = Cast<AEightBallPlayerState>(Player);
        if (PoolPlayer)
            PoolPlayer->AssignBallType(FBallType::NotInitialized);
    }

    const auto CurrentPlayer = TurnBasedPlayers[PlayerIndexTurn].GetObjectRef();
    const auto HandableBallPlayer = Cast<IPlayerWithHandableBall>(CurrentPlayer);

    if (FindAndInitializeCueBall())
    {
        HandableBallPlayer->SetBallInHand(CueBall, true);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Could not find a cue ball from EightBallGameState::OnFrameRestarted"));
    }
}

void AEightBallGameState::HandleTurnEnd_Internal()
{
    const auto PocketedBalls = BallsManager->GetBallsPocketedDuringTurn();
    const auto DroppedBalls = BallsManager->GetBallsDroppedDuringTurn();
    
    AEightBallGameMode* GM = Cast<AEightBallGameMode>(AuthorityGameMode);
    if (!ensure(GM != nullptr)) return;

    bool bNamedShotRuleActive = GM->bNamedShotRuleActive;
    bool bNamedShotSatisfied = bNamedShotRuleActive ? false : true;

    if (!RegisteredNamedShot.IsSet())
    {
        RegisteredNamedShot = PredictedNamedShot;
        const FString PredictedBall = PredictedNamedShot.SelectedBall ? PredictedNamedShot.SelectedBall->GetName() : "none";
        const FString PredictedPocket = PredictedNamedShot.SelectedPocket ? PredictedNamedShot.SelectedPocket->GetName() : "none";
        UE_LOG(LogTemp, Warning, TEXT("No named shot registered. Proceeding with predicted: ball %s, pocket %s"), *PredictedBall, *PredictedPocket);
    }

    /////////////////////////////
    // Loop over pocketed balls
    for (const auto& RawBall : PocketedBalls)
    {
        const auto Ball = Cast<ABallAmerican>(RawBall);
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
            CueBall = Ball;
            AssignFoul(FFoulReason::CueBallOut);
        }

        UPocketArea* lastPocket = RawBall->GetLastOverlappedPocket();
        if (lastPocket)
            UE_LOG(LogTemp, Warning, TEXT("Checking %s last overlapped pocket: %s"), *RawBall->GetName(), *lastPocket->GetName());
        
        if (RegisteredNamedShot.SelectedBall == RawBall &&
            RegisteredNamedShot.SelectedPocket == RawBall->GetLastOverlappedPocket())
        {
            UE_LOG(LogTemp, Warning, TEXT("Named/predicted shot is satisfied"));
            bNamedShotSatisfied = true;
        }

        if (!bTableOpened)
        {
            const auto Player = Cast<AEightBallPlayerState>(PlayerArray[PlayerIndexTurn]);
            FBallType CurrentPlayerBallType = Player->GetAssignedBallType();

            if (Ball->GetType() == CurrentPlayerBallType && bNamedShotSatisfied)
                bShouldSwitchTurn = false;
        }

    }

    //////////////////////////
    // Loop over dropped balls
    for (const auto& RawBall : DroppedBalls)
    {
        const auto Ball = Cast<ABallAmerican>(RawBall);
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
            CueBall = Ball;
            AssignFoul(FFoulReason::CueBallOut);
        }
    }

    if (bTableOpened &&
        PocketedBalls.Num() > 0)
    {
        bShouldSwitchTurn = false;
    }

    if (BallsManager->GetBallsHittedByTheCue().Num() == 0)
    {
        AssignFoul(FFoulReason::NoBallsHit);
    }

    if (!bTableOpened &&
        GM->bFirstTouchShouldBeFriendlyRuleActive &&
        BallsManager->GetBallsHittedByTheCue().Num() > 0)
    {
        const auto BallAm = Cast<ABallAmerican>(BallsManager->GetBallsHittedByTheCue()[0]);
        const auto Player = Cast<AEightBallPlayerState>(PlayerArray[PlayerIndexTurn]);
        if (Player->GetAssignedBallType() != BallAm->GetType())
            AssignFoul(FFoulReason::WrongBallTouchedFirst);
    }

    // assign balls type if not done yet
    if (PocketedBalls.Num() > 0 &&
        bBallsRackBroken &&
        bTableOpened &&
        FoulReason == FFoulReason::None &&
        bNamedShotSatisfied)
    {
        FBallType CurrentAssignedType = FBallType::NotInitialized,
            OtherAssignedType = FBallType::NotInitialized;
        for (const auto& Ball : PocketedBalls)
        {
            const auto AmericanBall = Cast<ABallAmerican>(Ball);
            if (AmericanBall->GetType() == FBallType::Solid)
            {
                CurrentAssignedType = FBallType::Solid;
                OtherAssignedType = FBallType::Stripe;
                break;
            }
            else if (AmericanBall->GetType() == FBallType::Stripe)
            {
                CurrentAssignedType = FBallType::Stripe;
                OtherAssignedType = FBallType::Solid;
                break;
            }
        }

        auto Player = Cast<AEightBallPlayerState>(PlayerArray[PlayerIndexTurn]);
        Player->AssignBallType(CurrentAssignedType);

        // if we are playing standalone, then do not reassign another type to the player
        if (PlayerArray.Num() > 1)
        {
            auto OtherPlayerIndex = (PlayerIndexTurn + 1) % PlayerArray.Num();
            Player = Cast<AEightBallPlayerState>(PlayerArray[OtherPlayerIndex]);
            Player->AssignBallType(OtherAssignedType);
        }

        // now the types are assigned
        bTableOpened = false;
    }
    
    if (!bBallsRackBroken && BallsManager->GetBallsHittedByTheCue().Num() > 0)
    {
        bBallsRackBroken = true;
    }
    

    if (FoulReason != FFoulReason::None)
    {
        if (!CueBall && !FindAndInitializeCueBall())
        {
            UE_LOG(LogPool, Warning, TEXT("Tried to give ball in hand, but CueBall is NULL"));
        }
        else
        {
            auto OtherPlayerIndex = (PlayerIndexTurn + 1) % PlayerArray.Num();
            GiveBallInHand(PlayerArray[OtherPlayerIndex], CueBall);
        }
    }

    if (bShouldSwitchTurn || FoulReason != FFoulReason::None)
    {
        SwitchTurn();
    }
    else
    {
        TurnBasedPlayers[PlayerIndexTurn]->SetIsMyTurn(true);
    }

    ClearTurnStateVariables();
    BroadcastOnTurnEnd();
}

bool AEightBallGameState::DecideWinCondition()
{
    // when 8 ball is scored and the rack was already broken,
    // there are two options: current player won or lose.

    // If balls of his types are all scored AND
    // on the 8ball pocketing he did not pocket any other ball,
    // then he won.
    // Otherwise - other player won.

    APoolGameMode* GM = nullptr;
    if (HasAuthority())
        GM = Cast<APoolGameMode>(AuthorityGameMode);
    // we return true cause in case we have some error
    // with getting GM and we return false, then the players
    // are stuck with the situation
    if (!ensure(GM != nullptr)) return true;

    auto PoolPlayer = Cast<AEightBallPlayerState>(PlayerArray[PlayerIndexTurn]);
    if (!ensure(PoolPlayer != nullptr)) return true;

    FBallType PlayersType = PoolPlayer->GetAssignedBallType();
    int32 BallsOfTypePlayedOut = 0;
    for (const auto& PlayedOutBall : BallsManager->GetBallsPlayedOut())
    {
        const auto AmericanBall = Cast<ABallAmerican>(PlayedOutBall);
        if (AmericanBall && AmericanBall->GetType() == PlayersType)
            ++BallsOfTypePlayedOut;
    }

    // if there are no balls of our type on the table
    if (BallsOfTypePlayedOut < GM->GetRequiredBallsToPocket())
        return false;
    // if we pocketed smth else otherwise than 8ball, we lose
    if (BallsManager->GetPocketedBalls().Num() > 1)
        return false;

    return true;
}

void AEightBallGameState::HandlePocketedBall(class ABall* Ball)
{
    Super::HandlePocketedBall(Ball);

    auto AmericanBall = Cast<ABallAmerican>(Ball);
    if (!ensure(AmericanBall != nullptr)) return;

    FBallType Type = AmericanBall->GetType();

    // for any ball we hide it in game
    // for cue just move it out of the way
    // until it is needed again when BallInHand.
    // We do not want to toggle visibility on and off
    // for cue ball as it is not necessary
    if (Type != FBallType::Cue)
    {
        auto BallComp = Cast<UStaticMeshComponent>(AmericanBall->GetRootComponent());
        BallComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    }
    else
    {
        CueBall = AmericanBall;
        CueBall->SetActorLocation(FVector(0, 0, 2000));
        CueBall->SetActorHiddenInGame(false);
    }
}

void AEightBallGameState::HandleBlackBallOutOfPlay()
{
    APoolGameMode* GM = Cast<APoolGameMode>(AuthorityGameMode);
    if (!ensure(GM != nullptr)) return;

    if (!bBallsRackBroken)
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
        WonPoolPlayer->Server_HandleFrameWon();
        NewFramesWon = WonPoolPlayer->GetFramesWon();

        if (NewFramesWon >= GM->RequiredFramesToWin)
            GM->EndMatch();
        else
            GM->RestartFrame();
    }
}

bool AEightBallGameState::FindAndInitializeCueBall()
{
    for (TActorIterator<ABallAmerican> It(GetWorld()); It; ++It)
    {
        if (It->GetType() == FBallType::Cue)
        {
            CueBall = *It;
            return true;
        }
    }
    return false;
}

void AEightBallGameState::Multicast_BoradcastPredictedShotRegistered_Implementation(FNamedShot PredictedShot)
{
    if (OnPredictedShotRegistered.IsBound())
        OnPredictedShotRegistered.Broadcast(PredictedShot);
}

void AEightBallGameState::Multicast_BroadcastNamedShotRegistered_Implementation(FNamedShot Shot)
{
    if (OnNamedShotRegistered.IsBound())
        OnNamedShotRegistered.Broadcast(Shot);
}

void AEightBallGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AEightBallGameState, CueBall);
}