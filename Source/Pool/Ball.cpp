// Fill out your copyright notice in the Description page of Project Settings.

#include "Ball.h"
#include "UnrealNetwork.h"

// Sets default values
ABall::ABall()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

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

// Called every frame
void ABall::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

}

