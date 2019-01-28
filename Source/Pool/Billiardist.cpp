// Fill out your copyright notice in the Description page of Project Settings.

#include "Billiardist.h"
#include "Components/InputComponent.h"
#include "UObject/UObjectIterator.h"
#include "GameFramework/CharacterMovementComponent.h"

UENUM(BlueprintType)
enum class FBilliardistState : uint8
{
    AIMING,    // when a ball is picked, we aim for the shot, holding the cue near the ball
    EXAMINING, // watching from the top of the table
    PICKING,   // if we are playing RU billiard, we can pick any ball for the shot
    WALKING    // just walking around the table, examining
};

// Sets default values
ABilliardist::ABilliardist()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ABilliardist::BeginPlay()
{
	Super::BeginPlay();

    // if no table assigned, we will try to look for one manually in the game world
    if (!m_pTable)
    {
        UE_LOG(LogTemp, Error, TEXT("Object %s has no table assigned. Looking for a table in the game world..."), *GetName());

        // try to find a table in the game world
        for (TObjectIterator<ATable> it; it; ++it)
        {
            ATable* foundTable = *it;
            if (foundTable)
            {
                UE_LOG(LogTemp, Error, TEXT("Found object %s. Assiging it to player %s"), *foundTable->GetName(), *GetName());
                m_pTable = foundTable;
                break; // we need any suitable table, so break after smth is found
            }
        }
    }

    // after we tried to find a table we can move the player on the spline
    if(m_pTable)
    {
        m_pSplinePath = m_pTable->GetSplinePath();
        if (m_pSplinePath)
        {
            SetActorLocation(m_pSplinePath->GetWorldLocationAtSplinePoint(0)); // place the actor in the beggining of the spline path around the table
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Object %s has no spline path (possibly no assigned billiard table.)"), *GetName());
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Object %s has no spline path (possibly no table assigned)"), *GetName()));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Object %s has no assigned table. Searching for a table in the game world did not succeed."), *GetName());
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Object %s has no assigned table"), *GetName()));
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
    
    FRotator Rotation = GetControlRotation();

    if (GetCharacterMovement()->IsFalling() || GetCharacterMovement()->IsMovingOnGround())
        Rotation.Pitch = 0.f;

    const FVector Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::X);
    
    m_fCurrentMoveDirection += Direction * Value;
    MoveAlongSpline();
}

void ABilliardist::MoveRight(float Value)
{
    if (!Controller || Value == 0.f)
        return;

    FRotator Rotation = GetControlRotation();

    const FVector Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::Y);

    m_fCurrentMoveDirection += Direction * Value;
    MoveAlongSpline();
}

void ABilliardist::MoveAlongSpline()
{
    m_fCurrentMoveDirection = m_fCurrentMoveDirection.GetSafeNormal(); // where would the character move without spline

    auto SplineTangent = m_pSplinePath->FindDirectionClosestToWorldLocation(GetActorLocation(), ESplineCoordinateSpace::World);

    auto cosin = FVector::DotProduct(SplineTangent, m_fCurrentMoveDirection) / 
        (SplineTangent.Size() * m_fCurrentMoveDirection.Size()); // cos between spline tangent and move direction without spline

    // if the cos is negative (so we are moving backwards on the spline) than negite-ness of cos will handle it
    auto ActualMovementVector = SplineTangent * cosin;

    AddMovementInput(ActualMovementVector.GetSafeNormal() * m_fMoveSpeed);
}