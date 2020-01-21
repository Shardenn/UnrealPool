// Copyright 2019 Andrei Vikarchuk.

#include "PoolGameState.h"

#include "Pool.h"
// TODO handle includes correctly, maybe refactor
//#include "Objects/Ball.h"

#include "Objects/Table/Table.h"
#include "Objects/Table/ActivePlayArea.h"
#include "Objects/Table/BallRegistrator.h"
#include "Objects/BallAmerican.h"

#include "AmericanPool/FrameStateEightBall.h"

#include "PoolPlayerState.h"
#include "PoolGameMode.h"

#include "UnrealNetwork.h"
#include "EngineUtils.h" // TObjectIterator
#include "Engine/ActorChannel.h"

APoolGameState::APoolGameState()
{
    FrameStateClass = UFrameStateEightBall::StaticClass();
    FrameState = NewObject<UFrameStateEightBall>(this, FrameStateClass);
}

bool APoolGameState::ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
    bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

    if (FrameState != nullptr)
    {
        WroteSomething |= Channel->ReplicateSubobject(FrameState, *Bunch, *RepFlags);
    }

    return WroteSomething;
}

void APoolGameState::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        APoolGameMode* GM = Cast<APoolGameMode>(AuthorityGameMode);
        if (!ensure(GM != nullptr)) return;

        GM->OnFrameRestart.AddDynamic(this, &APoolGameState::OnFrameRestarted);
    }
}

void APoolGameState::OnFrameRestarted()
{
    if (FrameState)
        FrameState->Restart();
}
/*
void APoolGameState::OnRep_UpdatePlayerStateTurn()
{
    uint32 FormerIndex = (PlayerIndexTurn - 1);
    if (FormerIndex < 0)
        FormerIndex += PlayerArray.Num();

    Cast<APoolPlayerState>(PlayerArray[FormerIndex])->SetIsMyTurn(false);
}*/

bool APoolGameState::SetPlayersReadyNum_Validate(uint32 PlayersReady) { return true; }
void APoolGameState::SetPlayersReadyNum_Implementation(uint32 PlayersReady)
{
    PlayersReadyNum = PlayersReady;
}

bool APoolGameState::FindAndInitializeCueBall()
{
    for (TActorIterator<ABallAmerican> It(GetWorld()); It; ++It)
    {
        if (It->GetType() == FBallType::Cue)
        {
            FrameState->SetCueBall(*It);
            return true;
        }
    }
    return false;
}

bool APoolGameState::Server_GiveBallInHand_Validate(APoolPlayerState* PlayerState) { return true; }
void APoolGameState::Server_GiveBallInHand_Implementation(APoolPlayerState* PlayerState)
{
    auto CueBall = FrameState->GetCueBall();
    if (!CueBall && !FindAndInitializeCueBall())
    {
        UE_LOG(LogPool, Warning, TEXT("GiveBallInHand: CueBall==nullptr. failed to find cue ball as well"));
        return;
    }

    auto BallPrimComp = Cast<UStaticMeshComponent>(CueBall->GetRootComponent());
    BallPrimComp->SetSimulatePhysics(false);
    CueBall->SetActorLocation(FVector(0, 0, 2000));

    Server_TakeBallFromHand();

    PlayerState->SetBallInHand(CueBall);
    //PlayerWithCueBall = PlayerState;

    //UE_LOG(LogPool, Warning, TEXT("gave ball to the player with index %d"), PlayerIndexTurn);
}
/*
bool APoolGameState::Server_TakeBallFromHand_Validate() { return true; }
void APoolGameState::Server_TakeBallFromHand_Implementation()
{
    if (PlayerWithCueBall)
    {
        PlayerWithCueBall->SetBallInHand(nullptr);
        PlayerWithCueBall = nullptr;
    }
}

*/
ABall* const APoolGameState::GetCueBall()
{
    /*if (!CueBall && !FindAndInitializeCueBall())
        return nullptr;
    return CueBall;*/
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



void APoolGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APoolGameState, PlayersReadyNum);
}