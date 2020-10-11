// Copyright 2019 Andrei Vikarchuk.

#include "Cue.h"

#include "Pool.h"

#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"

ACue::ACue()
{
    PrimaryActorTick.bCanEverTick = false;
    
    SetReplicates(true);
    // Movement is replicated via smooth sync component
    SetReplicateMovement(false);

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Cue mesh"));
    SetRootComponent(Mesh);

    OverlapTestCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Overlap test capsule"));
    OverlapTestCapsule->SetupAttachment(Mesh);
    
    CollisionTestStartingPoint = CreateDefaultSubobject<USceneComponent>(TEXT("Collision test start point"));
    CollisionTestStartingPoint->SetupAttachment(Mesh);
    CollisionTestEndPoint = CreateDefaultSubobject<USceneComponent>(TEXT("Collision test end point"));
    CollisionTestEndPoint->SetupAttachment(Mesh);
}

void ACue::BeginPlay()
{
    Super::BeginPlay();
}

void ACue::SubscribeToMeshOverlaps()
{
    Mesh->OnComponentBeginOverlap.AddDynamic(this, &ACue::OnMeshOverlap);
    Mesh->OnComponentEndOverlap.AddDynamic(this, &ACue::OnMeshEndOverlap);

    OverlapTestCapsule->OnComponentBeginOverlap.AddDynamic(this, &ACue::OnCapsuleOverlap);
    OverlapTestCapsule->OnComponentEndOverlap.AddDynamic(this, &ACue::OnCapsuleEndOverlap);
}

void ACue::OnMeshOverlap(UPrimitiveComponent* OverlappedComponent, 
    AActor* OtherActor, 
    UPrimitiveComponent* OtherComp, 
    int32 OtherBodyIndex, 
    bool bFromSweep, 
    const FHitResult& SweepResult)
{
    if (OtherComp == OverlapTestCapsule) return;
    MeshOverlappingComponents.AddUnique(OtherComp);
}

void ACue::OnMeshEndOverlap(UPrimitiveComponent* OverlappedComponent, 
    AActor* OtherActor, 
    UPrimitiveComponent* OtherComp, 
    int32 OtherBodyIndex)
{
    if (MeshOverlappingComponents.Contains(OtherComp))
        MeshOverlappingComponents.Remove(OtherComp);
}

void ACue::OnCapsuleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherComp == Mesh) return;
    CapsuleOverlappingComponents.AddUnique(OtherComp);
}

void ACue::OnCapsuleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (CapsuleOverlappingComponents.Contains(OtherComp))
        CapsuleOverlappingComponents.Remove(OtherComp);
}

void ACue::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ACue, MeshOverlappingComponents);
    DOREPLIFETIME(ACue, CapsuleOverlappingComponents);
}