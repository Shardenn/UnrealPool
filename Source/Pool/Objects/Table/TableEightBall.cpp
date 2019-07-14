// Copyright 2019 Andrei Vikarchuk.

#include "TableEightBall.h"
#include "Pool.h"
#include "EightBallSpawner.h"

ATableEightBall::ATableEightBall()
{
    BallSpawner->DestroyComponent();
    BallSpawner = CreateDefaultSubobject<UEightBallSpawner>(TEXT("Eight ball spawner"));
}

void ATableEightBall::BeginPlay()
{
    Super::BeginPlay();
}