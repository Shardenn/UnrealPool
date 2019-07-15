// Copyright 2019 Andrei Vikarchuk.


#include "BallAmerican.h"

#include "UnrealNetwork.h"

ABallAmerican::ABallAmerican() :
    ABall()
{}

void ABallAmerican::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ABallAmerican, BallNumber);
    DOREPLIFETIME(ABallAmerican, BallType);
}

void ABallAmerican::OnRep_BallNumber()
{
    SetupMaterial();
}
