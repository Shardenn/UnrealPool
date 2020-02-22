// Copyright 2019 Andrei Vikarchuk.

#include "PoolGameState.h"

#include "Pool.h"
// TODO handle includes correctly, maybe refactor
//#include "Objects/Ball.h"

#include "Objects/Table/Table.h"
#include "Objects/Table/ActivePlayArea.h"
#include "Objects/Table/BallRegistrator.h"
#include "Objects/BallAmerican.h"

#include "GameplayLogic/BallsManager.h"

#include "PoolPlayerState.h"
#include "PoolGameMode.h"

#include "UnrealNetwork.h"
#include "EngineUtils.h" // TObjectIterator
#include "Engine/ActorChannel.h"

APoolGameState::APoolGameState()
{
    bReplicates = true;
}

void APoolGameState::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        APoolGameMode* GM = Cast<APoolGameMode>(AuthorityGameMode);
        if (!ensure(GM != nullptr)) return;

        GM->OnFrameRestart.AddDynamic(this, &APoolGameState::OnFrameRestarted);

        // Objects only replicate from server to client. If we didn't guard this
        // the client would create the object just fine but it would get replaced
        // by the server version (more accurately the property would be replaced to
        // point to the version from the server. The one the client allocated would
        // eventually be garbage collected.
        BallsManager = NewObject<UBallsManager>(this); // NOTE: Very important, objects Outer must be our Actor!
    }
}

void APoolGameState::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    if (HasAuthority())
    {

    }
}

bool APoolGameState::ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
    bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

    if (BallsManager)
    {
        WroteSomething |= Channel->ReplicateSubobject(BallsManager, *Bunch, *RepFlags);
    }

    return WroteSomething;
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

UBallsManager* const APoolGameState::GetBallsManager()
{
    return BallsManager;
}

void APoolGameState::OnBallStartMoving(UPrimitiveComponent* Comp, FName BoneName)
{
    if (!bWatchBallsMovement)
        return;

    ABall* NewBall = Cast<ABall>(Comp->GetOwner());
    if (!NewBall || !BallsManager)
        return;

    BallsManager->AddMovingBall(NewBall);

    // Not on any ball added
    if (BallsManager->GetMovingBalls().Num() == 1)
    {
        APoolPlayerState* PlayerTurn = Cast<APoolPlayerState>(PlayerArray[PlayerIndexTurn]);
        PlayerTurn->SetIsMyTurn(false);
    }
}

void APoolGameState::OnBallStopMoving(UPrimitiveComponent* Comp, FName BoneName)
{
    if (!bWatchBallsMovement)
        return;
    
    ABall* Ball = Cast<ABall>(Comp->GetOwner());
    if (!Ball || !BallsManager)
        return;

    BallsManager->RemoveMovingBall(Ball);

    if (BallsManager->GetMovingBalls().Num() == 0)
    {
        bWatchBallsMovement = false;
        HandleTurnEnd();
    }
}

bool APoolGameState::SwitchTurn_Validate() { return true; }
void APoolGameState::SwitchTurn_Implementation()
{
    OnTurnEnd.Broadcast();

    APoolPlayerState* FormerPlayerTurn = Cast<APoolPlayerState>(PlayerArray[PlayerIndexTurn]);
    FormerPlayerTurn->SetIsMyTurn(false);

    PlayerIndexTurn = (PlayerIndexTurn + 1) % PlayerArray.Num();
    UE_LOG(LogPool, Warning, TEXT("Turn is on player indexed %d"), PlayerIndexTurn);
    APoolPlayerState* NewPlayerTurn = Cast<APoolPlayerState>(PlayerArray[PlayerIndexTurn]);
    NewPlayerTurn->SetIsMyTurn(true);
}

// Balls register event only on server,
//so the function is always executed on the server
void APoolGameState::OnBallOverlap(UPrimitiveComponent* OverlappedComponent, 
    AActor* OtherActor, 
    UPrimitiveComponent* OtherComp, 
    int32 OtherBodyIndex, 
    bool bFromSweep, 
    const FHitResult& SweepResult)
{
    UBallRegistrator* BallRegistratorComp = Cast<UBallRegistrator>(OtherComp);
    if (!BallRegistratorComp)
        return;

    ABallAmerican* PocketedBall = Cast<ABallAmerican>(OverlappedComponent->GetOwner());
    BallsManager->AddPocketedBall(PocketedBall);

    auto Comp = Cast<UStaticMeshComponent>(PocketedBall->GetRootComponent());
    OnBallStopMoving(Comp, NAME_None);
    Comp->SetSimulatePhysics(false);
    
    FBallType Type = PocketedBall->GetType();

    // for any ball we hide it in game
    // for cue just move it out of the way 
    // until it is needed again when BallInHand.
    // We do not want to toggle visibility on and off
    // for cue ball as it is not necessary
    if (Type != FBallType::Cue)
    {
        Comp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
        PocketedBall->SetActorHiddenInGame(true);
    }
    else
    {
        CueBall = PocketedBall;
        CueBall->SetActorLocation(FVector(0, 0, 2000));
    }
}

void APoolGameState::OnBallEndOverlap(UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor, 
    UPrimitiveComponent* OtherComp, 
    int32 OtherBodyIndex)
{
    UActivePlayArea* PlayArea = Cast<UActivePlayArea>(OtherComp);
    if (!PlayArea)
        return;

    ABallAmerican* DroppedBall = Cast<ABallAmerican>(OverlappedComponent->GetOwner());
    if (!DroppedBall)
        return;

    BallsManager->AddDroppedBall(DroppedBall);
    //OverlappedComponent->BodyInstance.bGenerateWakeEvents = false;
    OnBallStopMoving(OverlappedComponent, NAME_None);
}

void APoolGameState::OnCueBallHit(UPrimitiveComponent* HitComponent, 
    AActor* OtherActor, 
    UPrimitiveComponent* OtherComp, 
    FVector NormalImpulse, 
    const FHitResult& Hit)
{
    ABallAmerican* Ball = Cast<ABallAmerican>(OtherActor);
    if (!Ball) return;

    // TODO AddUnique?
    //BallsHittedByTheCue.Add(Ball);
}

void APoolGameState::OnFrameRestarted()
{
    ClearTurnStateVariables();

    CueBall = nullptr;

    bWatchBallsMovement = false;
    bTableOpened = true;
    bBallsRackBroken = false;

    BallsManager->Reset();

    SwitchTurn();

    for (auto& Player : PlayerArray)
    {
        APoolPlayerState* PoolPlayer = Cast<APoolPlayerState>(Player);
        if (PoolPlayer)
            PoolPlayer->AssignBallType(FBallType::NotInitialized);
    }
}

bool APoolGameState::HandleTurnEnd_Validate() { return true; }
void APoolGameState::HandleTurnEnd_Implementation()
{
    const auto PocketedBalls = BallsManager->GetPocketedBalls();
    const auto DroppedBalls = BallsManager->GetDroppedBalls();

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
            AssignFoul();
        }

        // TODO handle named shot

        if (!bTableOpened)
        {
            APoolPlayerState* Player = Cast<APoolPlayerState>(PlayerArray[PlayerIndexTurn]);
            FBallType CurrentPlayerBallType = Player->GetAssignedBallType();

            if (Ball->GetType() == CurrentPlayerBallType)
                bShouldSwitchTurn = false;
        }

    }

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
            UE_LOG(LogPool, Warning, TEXT("Cue ball is assigned: %s"), *CueBall->GetName());
            AssignFoul();
        }
    }

    if (bTableOpened && 
        PocketedBalls.Num() > 0)
    {
        bShouldSwitchTurn = false;
    }
    /*
    if (BallsHittedByTheCue.Num() == 0)
    {
        AssignFoul();
    }*/

    // assign balls type if not done yet
    if (PocketedBalls.Num() > 0 &&
        bBallsRackBroken &&
        bTableOpened &&
        !bPlayerFouled)
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
        bTableOpened = false;
    }
    /*
    if (BallsHittedByTheCue.Num() > 0)
    {
        if (!bBallsRackBroken)
            bBallsRackBroken = true;
    }
    */
    if (bShouldSwitchTurn || bPlayerFouled)
        SwitchTurn();
    else
    {
        // TODO move to other method
        APoolPlayerState* Player = Cast<APoolPlayerState>(PlayerArray[PlayerIndexTurn]);
        if (Player) 
            Player->SetIsMyTurn(true);
    }

    if (bPlayerFouled)
    {
        if (!CueBall && !FindAndInitializeCueBall())
        {
            UE_LOG(LogPool, Warning, TEXT("Tried to give ball in hand, but CueBall is NULL"));
        }
        else
        {
            Server_GiveBallInHand();
        }
    }

    ClearTurnStateVariables();
}

bool APoolGameState::AssignFoul_Validate() { return true; }
void APoolGameState::AssignFoul_Implementation()
{
    bPlayerFouled = true;
}

bool APoolGameState::DecideWinCondition()
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

    APoolPlayerState* PoolPlayer = Cast<APoolPlayerState>(PlayerArray[PlayerIndexTurn]);
    if (!ensure(PoolPlayer != nullptr)) return true;

    FBallType PlayersType = PoolPlayer->GetAssignedBallType();
    int32 BallsOfTypePlayedOut = 0;
    for (const auto& PlayedOutBall : BallsManager->GetBallsPlayedOut())
    {
        const auto AmericanBall = Cast<ABallAmerican>(PlayedOutBall);
        if (AmericanBall->GetType() == PlayersType)
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

void APoolGameState::ClearTurnStateVariables()
{
    bPlayerFouled = false;
    bShouldSwitchTurn = true;
}

void APoolGameState::HandleBlackBallOutOfPlay()
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
        WonPoolPlayer->HandleFrameWon();
        NewFramesWon = WonPoolPlayer->GetFramesWon();

        if (NewFramesWon >= GM->RequiredFramesToWin)
            GM->EndMatch();
        else
            GM->RestartFrame();
    }
}

bool APoolGameState::FindAndInitializeCueBall()
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

bool APoolGameState::Server_GiveBallInHand_Validate(APoolPlayerState* PlayerState) { return true; }
void APoolGameState::Server_GiveBallInHand_Implementation(APoolPlayerState* PlayerState)
{
    if (!CueBall && !FindAndInitializeCueBall())
    {
        UE_LOG(LogPool, Warning, TEXT("GiveBallInHand: CueBall==nullptr. failed to find cue ball as well"));
        return;
    }

    auto BallPrimComp = Cast<UStaticMeshComponent>(CueBall->GetRootComponent());
    BallPrimComp->SetSimulatePhysics(false);
    CueBall->SetActorLocation(FVector(0, 0, 2000));

    Server_TakeBallFromHand();
    
    // TODO it is debug feature, when we can pass nullptr to the func. Remove later?
    // if nullptr is given as parameter, we will automatically give a ball to the current player
    if (!PlayerState)
        PlayerState = Cast<APoolPlayerState>(PlayerArray[PlayerIndexTurn]);

    PlayerState->SetBallInHand(CueBall);
    PlayerWithCueBall = PlayerState;

    UE_LOG(LogPool, Warning, TEXT("gave ball to the player with index %d"), PlayerIndexTurn);
}

bool APoolGameState::Server_TakeBallFromHand_Validate() { return true; }
void APoolGameState::Server_TakeBallFromHand_Implementation()
{
    if (PlayerWithCueBall)
    {
        PlayerWithCueBall->SetBallInHand(nullptr);
        PlayerWithCueBall = nullptr;
    }
}

bool APoolGameState::IsMyTurn(const ITurnBasedPlayer* Player)
{
    if (BallsManager->GetMovingBalls().Num() > 0)
        return false;

    const auto CurrentPlayer = Cast<ITurnBasedPlayer>(PlayerArray[PlayerIndexTurn]);

    if (CurrentPlayer && CurrentPlayer == Player)
        return true;

    return false;
}

ABall* const APoolGameState::GetCueBall()
{
    if (!CueBall && !FindAndInitializeCueBall())
        return nullptr;
    return CueBall;
}

APoolPlayerState* APoolGameState::GetOtherPlayerState(const APoolPlayerState* Mine)
{
    for (auto Player : PlayerArray)
    {
        auto PoolPlayer = Cast<APoolPlayerState>(Player);
        if (PoolPlayer && PoolPlayer != Mine)
            return PoolPlayer;
    }
    return nullptr;
}

bool APoolGameState::Server_StartWatchingBallsMovement_Validate() { return true; }
void APoolGameState::Server_StartWatchingBallsMovement_Implementation()
{
    bWatchBallsMovement = true;
}

void APoolGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APoolGameState, PlayersReadyNum);
    DOREPLIFETIME(APoolGameState, PlayerIndexTurn);
    DOREPLIFETIME(APoolGameState, CueBall);
    DOREPLIFETIME(APoolGameState, BallsManager);
}