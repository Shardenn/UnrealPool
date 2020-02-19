// Copyright 2019 Andrei Vikarchuk.

#include "BallsManager.h"

void UBallsManager::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UBallsManager, MovingBalls);
    DOREPLIFETIME(UBallsManager, ActiveBalls);
    DOREPLIFETIME(UBallsManager, PocketedBalls);
    DOREPLIFETIME(UBallsManager, BallsHittedByTheCue);
    DOREPLIFETIME(UBallsManager, DroppedBalls);
    DOREPLIFETIME(UBallsManager, BallsPlayedOutOfGame);
}

void UBallsManager::Reset()
{
    MovingBalls.Empty();
    ActiveBalls.Empty();
    PocketedBalls.Empty();
    BallsHittedByTheCue.Empty();
    DroppedBalls.Empty();
    BallsPlayedOutOfGame.Empty();
}

void UBallsManager::AddMovingBall(class ABall* Ball)
{
    if (!Ball)
        return;

    MovingBalls.AddUnique(Ball);
}

void UBallsManager::RemoveMovingBall(class ABall* Ball)
{
    if (MovingBalls.Contains(Ball))
        MovingBalls.Remove(Ball);
}

void UBallsManager::AddPocketedBall(class ABall* Ball)
{
    if (!Ball)
        return;

    PocketedBalls.AddUnique(Ball);
    BallsPlayedOutOfGame.AddUnique(Ball);
    OnBallPlayedOut.Broadcast(Ball);
}

void UBallsManager::AddDroppedBall(class ABall* Ball)
{
    if (!Ball)
        return;

    DroppedBalls.AddUnique(Ball);
    BallsPlayedOutOfGame.AddUnique(Ball);
    OnBallPlayedOut.Broadcast(Ball);
}
