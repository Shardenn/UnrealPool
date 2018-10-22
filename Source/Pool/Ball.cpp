// Fill out your copyright notice in the Description page of Project Settings.

#include "Ball.h"


// Sets default values
ABall::ABall()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    m_pSphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere collision"));
    m_pSphereMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Sphere mesh"));
    RootComponent = m_pSphereCollision;
}

// Called when the game starts or when spawned
void ABall::BeginPlay()
{
    Super::BeginPlay();

    if (!m_pSphereCollision || !m_pSphereMesh)
        UE_LOG(LogTemp, Error, TEXT("Object %s has no either sphere mesh or collision."), *GetName());
}

// Called every frame
void ABall::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

}

