// Fill out your copyright notice in the Description page of Project Settings.

#include "Ball.h"

// Sets default values
ABall::ABall()
{
    bReplicates = true;

    m_pSphereMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Sphere mesh"));
    RootComponent = m_pSphereMesh;
}

// Called when the game starts or when spawned
void ABall::BeginPlay()
{
    Super::BeginPlay();

    if (!m_pSphereMesh)
        UE_LOG(LogTemp, Error, TEXT("Object %s has no sphere mesh."), *GetName());

    m_pSphereMesh->SetSimulatePhysics(true);
}
