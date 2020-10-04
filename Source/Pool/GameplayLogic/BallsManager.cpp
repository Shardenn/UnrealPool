// Copyright 2019 Andrei Vikarchuk.

#include "BallsManager.h"

#include "GameFramework/Actor.h"

#include "Engine/Engine.h"
#include "Engine/NetDriver.h"


void UBallsManager::Reset()
{
    MovingBalls.Empty();
    ActiveBalls.Empty();
    PocketedBalls.Empty();
    BallsHittedByTheCue.Empty();
    DroppedBalls.Empty();
    BallsPlayedOutOfGame.Empty();
}

void UBallsManager::OnTurnEnd()
{
    PocketedBalls.Append(BallsPocketedDuringTurn);
    DroppedBalls.Append(BallsDroppedDuringTurn);
    ResetTurnArrays();
}

// This gets called on server
void UBallsManager::OnFrameRestarted()
{
    Reset();

    AActor* owner = Cast<AActor>(GetOuter());
    if (owner == nullptr)
        return;
    
    Client_OnFrameRestarted();
}

void UBallsManager::Client_OnFrameRestarted_Implementation()
{
    UE_LOG(LogTemp, Warning, TEXT("Client_OnFrameRestarted fired"));
}

void UBallsManager::ResetTurnArrays()
{
    BallsPocketedDuringTurn.Empty();
    BallsDroppedDuringTurn.Empty();
    BallsHittedByTheCue.Empty();
}

void UBallsManager::AddMovingBall(ABall* Ball)
{
    if (!Ball)
        return;

    MovingBalls.AddUnique(Ball);
}

void UBallsManager::RemoveMovingBall(ABall* Ball)
{
    if (MovingBalls.Contains(Ball))
        MovingBalls.Remove(Ball);
}
// Fires on server
void UBallsManager::AddBallPocketedDuringTurn(ABall* Ball)
{
    if (!Ball)
        return;

    BallsPocketedDuringTurn.AddUnique(Ball);
    BallsPlayedOutOfGame.AddUnique(Ball);

    Multicast_OnBallPocketed(Ball);
}
// Fires on server
void UBallsManager::AddBallDroppedDuringTurn(ABall* Ball)
{
    if (!Ball)
        return;

    BallsDroppedDuringTurn.AddUnique(Ball);
    BallsPlayedOutOfGame.AddUnique(Ball);

    Multicast_OnBallPocketed(Ball);
}

void UBallsManager::AddBallHittedByTheCue(ABall* Ball)
{
    BallsHittedByTheCue.Add(Ball);
}

void UBallsManager::Multicast_OnBallPocketed_Implementation(const ABall* Ball)
{
    OnBallPocketed.Broadcast(Ball);
}

void UBallsManager::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UBallsManager, MovingBalls);
    DOREPLIFETIME(UBallsManager, ActiveBalls);
    DOREPLIFETIME(UBallsManager, PocketedBalls);
    DOREPLIFETIME(UBallsManager, BallsHittedByTheCue);
    DOREPLIFETIME(UBallsManager, DroppedBalls);
    DOREPLIFETIME(UBallsManager, BallsPlayedOutOfGame);
    DOREPLIFETIME(UBallsManager, BallsPocketedDuringTurn);
    DOREPLIFETIME(UBallsManager, BallsDroppedDuringTurn);
}

bool UBallsManager::CallRemoteFunction(UFunction* Function, void* Params, FOutParmRec* OutParms, FFrame* Stack)
{
    bool bProcessed = false;

    if (const auto MyOwner = Cast<AActor>(GetOuter()))
    {
        FWorldContext* const Context = GEngine->GetWorldContextFromWorld(GetWorld());
        if (Context != nullptr)
        {
            for (FNamedNetDriver& Driver : Context->ActiveNetDrivers)
            {
                if (Driver.NetDriver != nullptr && Driver.NetDriver->ShouldReplicateFunction(MyOwner, Function))
                {
                    Driver.NetDriver->ProcessRemoteFunction(MyOwner, Function, Params, OutParms, Stack, this);
                    bProcessed = true;
                }
            }
        }
    }

    return bProcessed;
}

int32 UBallsManager::GetFunctionCallspace(UFunction* Function, FFrame* Stack)
{
    return (GetOuter() ? GetOuter()->GetFunctionCallspace(Function, Stack) : FunctionCallspace::Local);
}