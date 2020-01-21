// Copyright 2019 Andrei Vikarchuk.


#include "TurnState.h"

#include "Objects/Ball.h"
#include "Objects/Table/BallRegistrator.h"
#include "Objects/Table/ActivePlayArea.h"

#include "UnrealNetwork.h"

UTurnState::UTurnState()
{

}

UTurnState::~UTurnState()
{

}

void UTurnState::ClearTurnStateVariables()
{
    m_bPlayerFouled = false;
    m_BallsHittedByTheCue.Empty();
    m_MovingBalls.Empty();
}

//bool UTurnState::Server_AssignFoul_Validate() { return true; }
void UTurnState::Server_AssignFoul()//_Implementation()
{
    m_bPlayerFouled = true;
}

void UTurnState::StartWatchingBallsMovement()
{
    m_bWatchBallsMovement = true;
}

void UTurnState::AddMovingBall(const UPrimitiveComponent* Comp, FName BoneName)
{
    if (!m_bWatchBallsMovement)
        return;

    ABall* NewBall = Cast<ABall>(Comp->GetOwner());
    if (!NewBall)
        return;

    m_MovingBalls.AddUnique(NewBall);

    OnBallStartsMoving.Broadcast(NewBall);
}

void UTurnState::RemoveMovingBall(const UPrimitiveComponent* Comp, FName BoneName)
{
    if (!m_bWatchBallsMovement)
        return;

    ABall* Ball = Cast<ABall>(Comp->GetOwner());
    if (!Ball)
        return;

    if (m_MovingBalls.Contains(Ball))
        m_MovingBalls.Remove(Ball);

    if (m_MovingBalls.Num() == 0)
    {
        m_bWatchBallsMovement = false;
        Server_HandleTurnEnd();
    }
}

// Balls register event only on server,
//so the function is always executed on the server
void UTurnState::OnBallOverlap(UPrimitiveComponent* OverlappedComponent,
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
    if (PocketedBall)
        m_BallsPocketed.Add(PocketedBall);

    auto Comp = Cast<UStaticMeshComponent>(PocketedBall->GetRootComponent());
    RemoveMovingBall(Comp, NAME_None);
    Comp->SetSimulatePhysics(false);

    HandleBallPocketed();
}

void UTurnState::HandleBallPocketed()
{
    /*
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
    */
}

void UTurnState::OnBallEndOverlap(UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex)
{
    UActivePlayArea* PlayArea = Cast<UActivePlayArea>(OtherComp);
    if (!PlayArea)
        return;

    ABall* DroppedBall = Cast<ABall>(OverlappedComponent->GetOwner());
    if (!DroppedBall)
        return;

    if (!m_BallsPocketed.Contains(DroppedBall))
        m_BallsDropped.Add(DroppedBall);
    //OverlappedComponent->BodyInstance.bGenerateWakeEvents = false;
    RemoveMovingBall(OverlappedComponent, NAME_None);
}

void UTurnState::OnCueBallHit(UPrimitiveComponent* HitComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    FVector NormalImpulse,
    const FHitResult& Hit)
{
    ABall* Ball = Cast<ABall>(OtherActor);
    if (!Ball) return;

    m_BallsHittedByTheCue.Add(Ball);
}

/*
void UTurnState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    //DOREPLIFETIME(UTurnState, PlayerIndexTurn);
}*/