// Copyright 2019 Andrei Vikarchuk.

#include "PoolGameState.h"

#include "Pool.h"
// TODO handle includes correctly, maybe refactor
//#include "Objects/Ball.h"

#include "PoolGameInstance.h"
#include "Objects/Table/Table.h"
#include "Objects/Table/Components/ActivePlayArea.h"
#include "Objects/Table/Components/BallRegistrator.h"
#include "Objects/Ball.h"

#include "GameplayLogic/BallsManager.h"

#include "GameplayLogic/PoolPlayerState.h"
#include "GameplayLogic/PoolGameMode.h"

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

        FramesToWin = GM->RequiredFramesToWin;
    }
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
        TurnBasedPlayers[PlayerIndexTurn]->SetIsMyTurn(false);
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

    ABall* PocketedBall = Cast<ABall>(OverlappedComponent->GetOwner());
    if (PocketedBall && !PocketedBall->IsActorBeingDestroyed() && PocketedBall->IsInGame())
        HandlePocketedBall(PocketedBall);
}

void APoolGameState::OnBallEndOverlap(UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor, 
    UPrimitiveComponent* OtherComp, 
    int32 OtherBodyIndex)
{
    UActivePlayArea* PlayArea = Cast<UActivePlayArea>(OtherComp);
    if (!PlayArea)
        return;

    ABall* DroppedBall = Cast<ABall>(OverlappedComponent->GetOwner());
    if (!DroppedBall || DroppedBall->IsActorBeingDestroyed() || !DroppedBall->IsInGame())
        return;

    BallsManager->AddBallDroppedDuringTurn(DroppedBall);
    OnBallStopMoving(OverlappedComponent, NAME_None);
}

void APoolGameState::OnCueBallHit(UPrimitiveComponent* HitComponent, 
    AActor* OtherActor, 
    UPrimitiveComponent* OtherComp, 
    FVector NormalImpulse, 
    const FHitResult& Hit)
{
    ABall* Ball = Cast<ABall>(OtherActor);
    if (!Ball) return;

    BallsManager->AddBallHittedByTheCue(Ball);
}

void APoolGameState::OnFrameRestarted()
{
    Multicast_BroadcastFrameRestartFired();

    ClearTurnStateVariables();

    bWatchBallsMovement = false;
    bTableOpened = true;
    bBallsRackBroken = false;

    BallsManager->OnFrameRestarted();

    SwitchTurn();
}

void APoolGameState::HandleTurnEnd()
{
    Server_HandleTurnEnd();
    if (GetLocalRole() < ROLE_Authority)
    {
    }
}

void APoolGameState::Server_HandleTurnEnd_Implementation()
{
    HandleTurnEnd_Internal();
}

bool APoolGameState::Server_HandleTurnEnd_Validate()
{
    return true;
}

void APoolGameState::HandleTurnEnd_Internal()
{
    ClearTurnStateVariables();
}

void APoolGameState::AssignFoul(const FFoulReason Reason)
{
    Server_AssignFoul(Reason);
}

FFoulReason APoolGameState::GetFoulReason() { return FoulReason; }

void APoolGameState::Server_AssignFoul_Implementation(const FFoulReason Reason)
{
    AssignFoul_Internal(Reason);
}

bool APoolGameState::Server_AssignFoul_Validate(const FFoulReason Reason) { return true; }

void APoolGameState::AssignFoul_Internal(const FFoulReason Reason)
{
    bPlayerFouled = Reason == FFoulReason::None ? false : true;
    FoulReason = Reason;
    Multicast_BroadcastPlayerFouled(FoulReason);
}

bool APoolGameState::DecideWinCondition()
{
    return true;
}

void APoolGameState::HandlePocketedBall(ABall* PocketedBall)
{
    BallsManager->AddBallPocketedDuringTurn(PocketedBall);

    auto Comp = Cast<UStaticMeshComponent>(PocketedBall->GetRootComponent());
    OnBallStopMoving(Comp, NAME_None);
    PocketedBall->RemoveBallFromGame();
}

void APoolGameState::ClearTurnStateVariables()
{
    AssignFoul(FFoulReason::None);
    bShouldSwitchTurn = true;
    BallsManager->OnTurnEnd();
}


void APoolGameState::Multicast_BroadcastFrameRestartFired_Implementation()
{
    if (OnFrameRestartFired.IsBound())
        OnFrameRestartFired.Broadcast();
}

bool APoolGameState::Multicast_BroadcastFrameRestartFired_Validate() { return true; }

void APoolGameState::Multicast_BroadcastPlayerFouled_Implementation(const FFoulReason Reason)
{
    if (OnPlayerFouled.IsBound())
        OnPlayerFouled.Broadcast(Reason);
}

bool APoolGameState::Multicast_BroadcastPlayerFouled_Validate(const FFoulReason Reason) { return true; }

bool APoolGameState::IsMyTurn(const TScriptInterface<ITurnBasedPlayer>& Player)
{
    if (BallsManager->GetMovingBalls().Num() > 0)
        return false;

    return Super::IsMyTurn(Player);
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
    DOREPLIFETIME(APoolGameState, BallsManager);
    DOREPLIFETIME(APoolGameState, FramesToWin);
}

void APoolGameState::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    if (HasAuthority())
    {
        // Objects only replicate from server to client. If we didn't guard this
        // the client would create the object just fine but it would get replaced
        // by the server version (more accurately the property would be replaced to
        // point to the version from the server. The one the client allocated would
        // eventually be garbage collected.
        BallsManager = NewObject<UBallsManager>(this); // NOTE: Very important, objects Outer must be our Actor!
        if (!BallsManager)
        {
            UE_LOG(LogPool, Error, TEXT("Failed to create BallsManager in PoolGameState"));
            return;
        }
        else
        {
            OnTurnEnd.AddDynamic(BallsManager, &UBallsManager::OnTurnEnd);
        }
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