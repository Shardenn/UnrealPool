// Fill out your copyright notice in the Description page of Project Settings.

#include "Billiardist.h"
#include "Components/InputComponent.h"
#include "UObject/UObjectIterator.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UnrealNetwork.h"

#include "DrawDebugHelpers.h"

#include "Ball.h"
#include "Components/ActorComponent.h"
#include "Camera/CameraComponent.h"
#include "Pool.h"



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

bool ABilliardist::Server_SubscribeToStateChange_Validate() { return true; }
void ABilliardist::Server_SubscribeToStateChange_Implementation()
{
    auto controller = Cast<ABilliardistController>(GetController());
    if (controller)
        controller->OnStateChange.AddDynamic(this, &ABilliardist::OnPlayerStateChanged);
}


// Called when the game starts or when spawned
void ABilliardist::BeginPlay()
{
    Super::BeginPlay();
    Server_SubscribeToStateChange();
}

void ABilliardist::OnPlayerStateChanged(FBilliardistState newState)
{
    UE_LOG(LogPool, Warning, TEXT("%s sees that player state changed to %d"),
        *GetName(), static_cast<uint8>(newState));
}

// Called every frame
void ABilliardist::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    FBilliardistState currState = FBilliardistState::WALKING;
    if (Cast<ABilliardistController>(GetController()) != nullptr)
    {
        currState = Cast<ABilliardistController>(GetController())->GetState();

        switch (currState)
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
                // update the hit strength


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
            UE_LOG(LogPool, Error, TEXT("%s: Tried to tell controller %s to update the table but failed - got null on controller cast"),
                *GetName(),
                *GetController()->GetName());
        }
        else
        {
            UE_LOG(LogPool, Error, TEXT("%s: does not have controller at all. What?"), *GetName());
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

