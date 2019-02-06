// Fill out your copyright notice in the Description page of Project Settings.

#include "Billiardist.h"
#include "Components/InputComponent.h"
#include "UObject/UObjectIterator.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UnrealNetwork.h"
#include "Ball.h"
#include "BilliardistController.h"

#ifndef STATE_MACHINE
#define STATE_MACHINE
// observing is the state that takes place after a hit - when we are waiting for the balls to stop
int BillStateMachine[5][5] = { // state machine of transferring from one state to another
    // W, P, A, O, E
    { 1, 1, 1, 0, 1 }, // Walking
    { 1, 1, 1, 0, 1 }, // Picking
    { 0, 1, 1, 1, 1 }, // Aiming - cant return directly to moving
    { 0, 0, 0, 1, 1 }, // Observing - cant return to any state
    { 1, 1, 1, 1, 1 }  // Examining
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
    DOREPLIFETIME(ABilliardist, m_eState);
}

// Called when the game starts or when spawned
void ABilliardist::BeginPlay()
{
    Super::BeginPlay();
}

// Called every frame
void ABilliardist::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    switch (m_eState)
    {
        case FBilliardistState::WALKING:
        {
            break;
        }
        case FBilliardistState::PICKING:
        {
            // highlight a ball that may be picked right now
            break;
        }
        case FBilliardistState::AIMING:
        {
            // update the hot strength
            break;
        }
        case FBilliardistState::OBSERVING:
        {
            break;
        }
        case FBilliardistState::EXAMINING:
        {
            break;
        }
    }
}

// Called to bind functionality to input
void ABilliardist::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAxis("MoveForward", this, &ABilliardist::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &ABilliardist::MoveRight);
    PlayerInputComponent->BindAxis("Turn", this, &ABilliardist::AddControllerYawInput);
    PlayerInputComponent->BindAxis("LookUp", this, &ABilliardist::AddControllerPitchInput);

    PlayerInputComponent->BindAction("Action", IE_Pressed, this, &ABilliardist::ActionPressHandle);
    PlayerInputComponent->BindAction("Return", IE_Pressed, this, &ABilliardist::ReturnPressHandle);

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
}

void ABilliardist::MoveRight(float Value)
{
    if (!Controller || Value == 0.f)
        return;

    auto Rotation = GetControlRotation();

    const auto Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::Y);

    m_fCurrentMoveDirection += Direction * Value;
}

// when action button is pressed, here we check the next possible state to switch
void ABilliardist::ActionPressHandle()
{
    switch (m_eState)
    {
        case FBilliardistState::WALKING:
        {
            SetState(FBilliardistState::PICKING);
            break;
        }
        case FBilliardistState::PICKING:
        {
            // when we press LMB while PIKING, we should 
            // 1. set the selected ball
            for (TObjectIterator<ABall> it; it; ++it)
            {
                auto FoundBall = *it;
                if (FoundBall)
                {
                    Cast<ABilliardistController>(GetController())->SetBall(FoundBall);
                }
            }
            SetState(FBilliardistState::AIMING);
            // 2. blend the camera (in controller actually) to it
            break;
        }
        case FBilliardistState::AIMING:
        {
            // set observing
            // 1. get the current hit strength
            // 2. handle ball push
            // 3. in this state it is possible to switch between additional cameras
            break;
        }
        case FBilliardistState::OBSERVING:
        {
            // set any state, but it is possible only to set examining (handled in setstate)
            break;
        }
        case FBilliardistState::EXAMINING:
        {
            // return to the previous state
            break;
        }
    }
}

// when return button is pressed, here we check the next possible state to switch
void ABilliardist::ReturnPressHandle()
{
    switch (m_eState)
    {
        case FBilliardistState::WALKING:
        {
            // nowhere to return, it is a default state
            break;
        }
        case FBilliardistState::PICKING:
        {
            SetState(FBilliardistState::WALKING);
            break;
        }
        case FBilliardistState::AIMING:
        {
            // 1. set picking
            // 2. clear selected ball
            break;
        }
        case FBilliardistState::OBSERVING:
        {
            // set examining
            // we cannot return to anything except examining
            break;
        }
        case FBilliardistState::EXAMINING:
        {
            // return to the previous state
            break;
        }
    }
}

void ABilliardist::SetTable(ATable* NewTable)
{
    Server_SetTable(NewTable);
    auto GotController = Cast<ABilliardistController>(GetController());
    if (GotController)
    {
        GotController->SetTable(NewTable);
    }
    else
    {
        if (GetController())
        {
            UE_LOG(LogTemp, Error, TEXT("%s: Tried to tell controller %s to update the table but failed - got null on controller cast"),
                *GetName(),
                *GetController()->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("%s: does not have controller at all. What?"), *GetName());
        }
    }
}

bool ABilliardist::Server_SetTable_Validate(ATable*) { return true; }

void ABilliardist::Server_SetTable_Implementation(ATable* NewTable)
{
    m_pTable = NewTable;
    if (m_pTable)
        m_pSplinePath = m_pTable->GetSplinePath();
    else
    {
        m_pSplinePath = nullptr;
        UE_LOG(LogTemp, Warning, TEXT("%s was assigned with the null table and therefore null spline path"), *GetName());
    }
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
    {
        m_eState = NewState;
        OnStateChange.Broadcast(m_eState);
    }
}