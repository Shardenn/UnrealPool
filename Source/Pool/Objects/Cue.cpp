// Copyright 2019 Andrei Vikarchuk.

#include "Cue.h"

#include "Components/StaticMeshComponent.h"

ACue::ACue()
{
    PrimaryActorTick.bCanEverTick = false;
    
    SetReplicates(true);
    // Movement is replicated via smooth sync component
    SetReplicateMovement(false);

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Cue mesh"));
    SetRootComponent(Mesh);
}