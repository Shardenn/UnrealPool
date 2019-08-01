// Copyright 2019 Andrei Vikarchuk.

#include "BallAmerican.h"
#include "AmericanPool/PoolGameState.h"

#include "Kismet/GameplayStatics.h"
#include "UnrealNetwork.h"

ABallAmerican::ABallAmerican() :
    ABall()
{}

void ABallAmerican::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        if (BallType == FBallType::Cue)
        {
            APoolGameState* GameState = Cast<APoolGameState>(UGameplayStatics::GetGameState(GetWorld()));
            if (!ensure(GameState != nullptr)) return;

            SphereMesh->OnComponentHit.AddDynamic(GameState, &APoolGameState::OnCueBallHit);
        }
    }
}

void ABallAmerican::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ABallAmerican, BallNumber);
    DOREPLIFETIME(ABallAmerican, BallType);
}

bool ABallAmerican::SetBallNumber_Validate(uint8 Number) { return true; }
void ABallAmerican::SetBallNumber_Implementation(uint8 Number)
{
    if (Number < 8)
        SetBallType(FBallType::Solid);
    else if (Number == 8)
        SetBallType(FBallType::Black);
    else if (Number <= 15)
        SetBallType(FBallType::Stripe);
    else if (Number == 16) // cue ball
        SetBallType(FBallType::Cue);

    BallNumber = Number;
    SetupColorAndNumber();
}

bool ABallAmerican::SetBallType_Validate(FBallType Type) { return true; }
void ABallAmerican::SetBallType_Implementation(FBallType Type)
{
    BallType = Type;
    SetupStripeness();
}

void ABallAmerican::OnRep_BallNumber()
{
    SetupColorAndNumber();
}

void ABallAmerican::OnRep_BallType()
{
    SetupStripeness();
}
