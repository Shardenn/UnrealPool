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

    UFUNCTION(BlueprintCallable)
    void SetTable(ATable* NewTable);
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

    // does not needs to be replicated for movement (only m_pSpline does)
    // but may be needed for replication later
    UPROPERTY(/*Replicated, */EditAnywhere, meta = (DisplayName = "Assigned billiard table"))
    ATable* m_pTable = nullptr;
    UPROPERTY(EditAnywhere, meta = (DisplayName = "Move speed"))
    float m_fMoveSpeed = 1.0f;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    UFUNCTION(BlueprintPure, meta = (DisplayName = "Get Move Speed"))
    float GetMoveSpeed() { return m_fMoveSpeed; }
    UFUNCTION(BlueprintPure, meta = (DisplayName = "Get Spline"))
    USplineComponent* GetSpline() { return m_pSplinePath; }
    //UPROPERTY(Replicated)
    FVector m_fCurrentMoveDirection = FVector(0);
private:
    UFUNCTION()
    void MoveForward(float Value);
    UFUNCTION()
    void MoveRight(float Value);
    UPROPERTY(Replicated) // needed for replication, tested
    USplineComponent* m_pSplinePath = nullptr;
    
    UFUNCTION(server, reliable, WithValidation)
    void Server_SetTable(ATable* NewTable);
};
