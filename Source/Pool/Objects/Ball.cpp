#include "Ball.h"
#include "Pool.h"
#include "AmericanPool/PoolGameState.h"

#include "Kismet/GameplayStatics.h"

// Sets default values
ABall::ABall()
{
    bReplicates = true;

    SphereMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Sphere mesh"));
    SetRootComponent(SphereMesh);
    
    SphereMesh->SetSimulatePhysics(true);
    SphereMesh->BodyInstance.bGenerateWakeEvents = true;
}

// Called when the game starts or when spawned
void ABall::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        GameState = Cast<APoolGameState>(UGameplayStatics::GetGameState(GetWorld()));
        if (!ensure(GameState != nullptr)) return;

        SphereMesh->OnComponentWake.AddDynamic(GameState, &APoolGameState::AddMovingBall);
        SphereMesh->OnComponentSleep.AddDynamic(GameState, &APoolGameState::RemoveMovingBall);
    }
}