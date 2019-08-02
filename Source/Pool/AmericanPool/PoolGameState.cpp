#include "PoolGameState.h"

#include "Pool.h"
// TODO handle includes correctly, maybe refactor
//#include "Objects/Ball.h"
#include "Objects/BallAmerican.h"
#include "Objects/Table/Table.h"
#include "PoolPlayerState.h"

#include "UnrealNetwork.h"
#include "Components/BoxComponent.h"
#include "Objects/Table/BallRegistrator.h"

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
    if (!bWatchBallsMovement)
        return;

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
    if (!bWatchBallsMovement)
        return;
    
    ABall* Ball = Cast<ABall>(Comp->GetOwner());
    if (!Ball)
        return;

    if (MovingBalls.Contains(Ball))
        MovingBalls.Remove(Ball);

    if (MovingBalls.Num() == 0)
    {
        bWatchBallsMovement = false;
        //SwitchTurn();
        HandleTurnEnd();
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

void APoolGameState::OnBallOverlap(UPrimitiveComponent* OverlappedComponent, 
    AActor* OtherActor, 
    UPrimitiveComponent* OtherComp, 
    int32 OtherBodyIndex, 
    bool bFromSweep, 
    const FHitResult& SweepResult)
{
    // TODO cast to BallRegistrator, not to BoxComponent
    UBoxComponent* BallRegistratorComp = Cast<UBoxComponent>(OtherComp);
    if (!BallRegistratorComp)
        return;

    UE_LOG(LogPool, Warning, TEXT("Ball %s overlapped with ball registrator %s"), *OverlappedComponent->GetOwner()->GetName(), 
        *BallRegistratorComp->GetName());

    ABallAmerican* PocketedBall = Cast<ABallAmerican>(OverlappedComponent->GetOwner());
    if (PocketedBall)
        PocketedBalls.Add(PocketedBall);

    auto Comp = Cast<UStaticMeshComponent>(PocketedBall->GetRootComponent());
    RemoveMovingBall(Comp, NAME_None);
    
    FBallType Type = PocketedBall->GetType();
    if (Type != FBallType::Cue &&
        Type != FBallType::Black)
    {
        Comp->SetSimulatePhysics(false);
        Comp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
        PocketedBall->SetActorHiddenInGame(true);
    }
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
    BallsHittedByTheCue.Add(Ball);
}

bool APoolGameState::HandleTurnEnd_Validate() { return true; }
void APoolGameState::HandleTurnEnd_Implementation()
{
    for (auto Ball : PocketedBalls)
    {
        if (Ball->GetType() == FBallType::Black)
        {
            // TODO handle black ball potting
            UE_LOG(LogPool, Warning, TEXT("The black ball was potted"));
        }
        else if (Ball->GetType() == FBallType::Cue)
        {
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

        RegisterBall(Ball);
    }

    if (BallsHittedByTheCue.Num() == 0)
    {
        AssignFoul();
    }

    // assign balls type if not done yet
    if (PocketedBalls.Num() > 0 &&
        bBallsRackBroken &&
        bTableOpened &&
        !bPlayerFouled)
    {
        FBallType CurrentAssignedType, OtherAssignedType;
        for (auto& Ball : PocketedBalls)
        {
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
        bTableOpened = false;
    }

    if (BallsHittedByTheCue.Num() > 0)
    {
        if (!bBallsRackBroken)
            bBallsRackBroken = true;
    }

    if (bShouldSwitchTurn)
        SwitchTurn();

    if (bPlayerFouled)
        GiveBallInHand();

    ClearTurnStateVariables();
}

bool APoolGameState::AssignFoul_Validate() { return true; }
void APoolGameState::AssignFoul_Implementation()
{
    bPlayerFouled = true;
}

bool APoolGameState::RegisterBall_Validate(ABallAmerican*) { return true; }
void APoolGameState::RegisterBall_Implementation(ABallAmerican* Ball)
{
    FBallType Type = Ball->GetType();
    if (Type != FBallType::Cue &&
        Type != FBallType::Black)
    ActiveBalls.Remove(Ball);
}

void APoolGameState::ClearTurnStateVariables()
{
    bPlayerFouled = false;
    bShouldSwitchTurn = true;

    PocketedBalls.Empty();
    BallsHittedByTheCue.Empty();
    DroppedBalls.Empty();
}

bool APoolGameState::GiveBallInHand_Validate(APoolPlayerState* PlayerState) { return true; }
void APoolGameState::GiveBallInHand_Implementation(APoolPlayerState* PlayerState)
{
    ABall* CueBall = nullptr;
    for (auto& Ball : ActiveBalls)
    {
        auto AmBall = Cast<ABallAmerican>(Ball);
        if (!AmBall)
            continue;
        if (AmBall->GetType() == FBallType::Cue)
            CueBall = AmBall;
    }

    if (!ensure(CueBall != nullptr)) return;
    auto BallPrimComp = Cast<UStaticMeshComponent>(CueBall->GetRootComponent());
    BallPrimComp->SetSimulatePhysics(false);
    CueBall->SetActorLocation(FVector(0, 0, 100));

    TakeBallFromHand();
    
    // TODO it is debug feature, when we can pass nullptr to the func. Remove later?
    // if nullptr is given as parameter, we will automatically give a ball to the current player
    if (!PlayerState)
        PlayerState = Cast<APoolPlayerState>(PlayerArray[PlayerIndexTurn]);

    PlayerState->SetBallInHand(CueBall);
    PlayerWithCueBall = PlayerState;

    UE_LOG(LogPool, Warning, TEXT("gave ball to the player with index %d"), PlayerIndexTurn);
}

bool APoolGameState::TakeBallFromHand_Validate() { return true; }
void APoolGameState::TakeBallFromHand_Implementation()
{
    if (PlayerWithCueBall)
    {
        PlayerWithCueBall->SetBallInHand(nullptr);
        PlayerWithCueBall = nullptr;
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