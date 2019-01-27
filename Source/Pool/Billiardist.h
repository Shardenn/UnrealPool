// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/Engine.h"
#include "Table.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SplineComponent.h"
#include "Billiardist.generated.h"

UCLASS()
class POOL_API ABilliardist : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABilliardist();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "Assigned billiard table"))
    ATable* m_pTable = nullptr;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
    UFUNCTION()
    void MoveForward(float Value);
    UFUNCTION()
    void MoveRight(float Value);

    FVector m_fCurrentMoveDirection = FVector(0);
    USplineComponent* m_pSplinePath = nullptr;
    
};
