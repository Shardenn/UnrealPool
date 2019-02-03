// Fill out your copyright notice in the Description page of Project Settings.

#include "Billiardist.h"
#include "Components/InputComponent.h"
#include "UObject/UObjectIterator.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UnrealNetwork.h"

#ifndef STATE_MACHINE
#define STATE_MACHINE
int BillStateMachine[5][5] = { // state machine of transferring from one state to another
    // W, P, A, O, E
    { 1, 1, 1, 0, 1 }, // Walking
    { 1, 1, 1, 0, 1 }, // Picking
    { 1, 1, 1, 0, 1 }, // Aiming
    { 1, 1, 1, 0, 1 }, // Observing
    { 1, 1, 1, 0, 1 }  // Examining
};
#endif

// Sets default values
ABilliardist::ABilliardist()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// VisualStudio shows IntelliSense error, but it actually compiles
void ABilliardist::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // Replicate to everyone
    //DOREPLIFETIME(ABilliardist, m_pTable);
    DOREPLIFETIME(ABilliardist, m_pSplinePath);
}

// Called when the game starts or when spawned
void ABilliardist::BeginPlay()
{
	Super::BeginPlay();

    // if no table assigned, we will try to look for one manually in the game world
    if (!m_pTable)
    {
        // warn if we do not have a table assigned
        UE_LOG(LogTemp, Error, TEXT("Object %s has no table assigned."), *GetName());
        if (GEngine)
            GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5.f, FColor::Red, FString::Printf(TEXT("Object %s has no spline path (possibly no table assigned)"), *GetName()));
    }
}

// Called every frame
void ABilliardist::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ABilliardist::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAxis("MoveForward", this, &ABilliardist::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &ABilliardist::MoveRight);
    PlayerInputComponent->BindAxis("Turn", this, &ABilliardist::AddControllerYawInput);
    PlayerInputComponent->BindAxis("LookUp", this, &ABilliardist::AddControllerPitchInput);

}

void ABilliardist::MoveForward(float Value)
{
    if (!Controller || Value == 0.f)
    {
        return;
    }
    
    auto Rotation = GetControlRotation();

    if (GetCharacterMovement()->IsFalling() || GetCharacterMovement()->IsMovingOnGround())
        Rotation.Pitch = 0.f;

    const auto Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::X);
    
    m_fCurrentMoveDirection += Direction * Value;
    //MoveAlongSpline();
}

void ABilliardist::MoveRight(float Value)
{
    if (!Controller || Value == 0.f)
        return;

    auto Rotation = GetControlRotation();

    const auto Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::Y);

    m_fCurrentMoveDirection += Direction * Value;
    //MoveAlongSpline();
}

void ABilliardist::SetTable(ATable* NewTable)
{
    Server_SetTable(NewTable);
}

bool ABilliardist::Server_SetTable_Validate(ATable*) { return true; }

void ABilliardist::Server_SetTable_Implementation(ATable* NewTable)
{
    m_pTable = NewTable;
    if (m_pTable)
        m_pSplinePath = m_pTable->GetSplinePath();
}

void ABilliardist::SetState(FBilliardistState NewState)
{
    Server_SetState(NewState);
}

bool ABilliardist::Server_SetState_Validate(FBilliardistState) { return true; }

void ABilliardist::Server_SetState_Implementation(FBilliardistState NewState)
{
    if (m_eState == NewState)
        return;

    if (BillStateMachine[(int)m_eState][(int)NewState] == 1) // only if state machine allows us the queried state transfer
                                                  // then we update the state. It is replicated automatically
                                                  // by UPROPERTY
        m_eState = NewState;
}