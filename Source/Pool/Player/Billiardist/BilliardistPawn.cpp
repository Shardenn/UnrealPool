// Copyright 2019 Andrei Vikarchuk.


#include "BilliardistPawn.h"

#include "BilliardistMovementComponent.h"

#include "Components/InputComponent.h"

ABilliardistPawn::ABilliardistPawn()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
    bReplicateMovement = true;

    MovementComponent = CreateDefaultSubobject<UBilliardistMovementComponent>(TEXT("Movement component"));
}

void ABilliardistPawn::BeginPlay()
{
    Super::BeginPlay();

}

void ABilliardistPawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

}

void ABilliardistPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAxis("MoveRight", this, &ABilliardistPawn::MoveRight);
    PlayerInputComponent->BindAxis("MoveForward", this, &ABilliardistPawn::MoveForward);
    //PlayerInputComponent->BindAxis("Turn", this, &ABilliardist::Turn);
    //PlayerInputComponent->BindAxis("LookUp", this, &ABilliardist::LookUp);

    PlayerInputComponent->BindAction("Return", IE_Pressed, this, &ABilliardistPawn::ReturnPressHandle);
    PlayerInputComponent->BindAction("Action", IE_Pressed, this, &ABilliardistPawn::ActionPressHandle);
    //PlayerInputComponent->BindAction("TopView", IE_Pressed, this, &ABilliardistPawn::ExaminingPressHandle);
    PlayerInputComponent->BindAction("Ready", IE_Pressed, this, &ABilliardistPawn::ReadyPressHandle);
}

void ABilliardistPawn::SetSpline(USplineComponent* Spline)
{
    if (!MovementComponent) return;
    MovementComponent->SetSpline(Spline);
}

void ABilliardistPawn::MoveForward(float Value)
{
    if (!MovementComponent) return;
    MovementComponent->SetForwardIntent(Value);
}

void ABilliardistPawn::MoveRight(float Value)
{
    if (!MovementComponent) return;
    MovementComponent->SetRightIntent(Value);
}

void ABilliardistPawn::ActionPressHandle()
{
}

void ABilliardistPawn::ReturnPressHandle()
{
}

void ABilliardistPawn::ReadyPressHandle()
{
}
