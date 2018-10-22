// Fill out your copyright notice in the Description page of Project Settings.

#include "BallRegistrator.h"


// Sets default values
ABallRegistrator::ABallRegistrator()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;
    m_pBoxCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("Ball register collision"));
    RootComponent = m_pBoxCollider;
}

// Called when the game starts or when spawned
void ABallRegistrator::BeginPlay()
{
    Super::BeginPlay();
    if (!m_pBoxCollider)
        UE_LOG(LogTemp, Error, TEXT("The object %s does not have a box collider component"), *GetName());
}

// Called every frame
void ABallRegistrator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

}

