// Copyright 2019 Andrei Vikarchuk.

#include "Ball.h"
#include "Pool.h"
#include "GameplayLogic/PoolGameState.h"

#include "Kismet/GameplayStatics.h"

// Sets default values
ABall::ABall()
{
    bReplicates = true;

    SphereMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Sphere mesh"));
    SetRootComponent(SphereMesh);
    
    SphereMesh->SetSimulatePhysics(true);
    SphereMesh->SetEnableGravity(true);
    SphereMesh->BodyInstance.bGenerateWakeEvents = true;
    //SphereMesh->SetMassOverrideInKg(NAME_None, 0.2);
    SphereMesh->SetAngularDamping(0.6);
    SphereMesh->SetLinearDamping(0.2);

    SphereMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    SphereMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
}

void ABall::RemoveBallFromGame()
{
    auto Comp = Cast<UStaticMeshComponent>(GetRootComponent());
    Comp->SetSimulatePhysics(false);

    SetActorHiddenInGame(true);
}

void ABall::ReturnBallIntoGame()
{
    auto Comp = Cast<UStaticMeshComponent>(GetRootComponent());
    Comp->SetSimulatePhysics(true);

    SetActorHiddenInGame(false);
}

// Called when the game starts or when spawned
void ABall::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        APoolGameState* GameState = Cast<APoolGameState>(UGameplayStatics::GetGameState(GetWorld()));
        if (!ensure(GameState != nullptr)) return;

        SphereMesh->OnComponentWake.AddDynamic(GameState, &APoolGameState::OnBallStartMoving);
        SphereMesh->OnComponentSleep.AddDynamic(GameState, &APoolGameState::OnBallStopMoving);

        SphereMesh->OnComponentBeginOverlap.AddDynamic(GameState, &APoolGameState::OnBallOverlap);
        SphereMesh->OnComponentEndOverlap.AddDynamic(GameState, &APoolGameState::OnBallEndOverlap);
    }
}