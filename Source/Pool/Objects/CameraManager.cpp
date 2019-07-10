// Fill out your copyright notice in the Description page of Project Settings.

#include "CameraManager.h"

// Sets default values
ACameraManager::ACameraManager()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void ACameraManager::BeginPlay()
{
    Super::BeginPlay();

}

// Called every frame
void ACameraManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

}

