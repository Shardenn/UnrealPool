// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/Engine.h"
#include "Table.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SplineComponent.h"
#include "Billiardist.generated.h"


UENUM(BlueprintType)
enum class FBilliardistState : uint8
{
    WALKING     UMETA(DisplayName = "Walking"),     // just walking around the table, examining
    PICKING     UMETA(DisplayName = "Picking"),     // if we are playing RU billiard, we can pick any ball for the shot
    AIMING      UMETA(DisplayName = "Aiming"),      // when a ball is picked, we aim for the shot, holding the cue near the ball
    OBSERVING   UMETA(DisplayName = "Observing"),  // observing the balls after a shot
    EXAMINING   UMETA(DisplayName = "Examinging"),  // watching from the top of the table
    POSSIBLE_STATES_NUMBER = 5 UMETA(DisplayName = "Possible values number")
};

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

    UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set State"))
    void SetState(FBilliardistState NewState);

    UFUNCTION(BlueprintPure, meta = (DisplayName = "GetState"))
    FBilliardistState GetState() { return m_eState; }
private:
    UFUNCTION()
    void MoveForward(float Value);
    UFUNCTION()
    void MoveRight(float Value);
    UPROPERTY(Replicated) // needed for replication, tested
    USplineComponent* m_pSplinePath { nullptr };
    
    UPROPERTY(Replicated)
    FBilliardistState m_eState { FBilliardistState::WALKING };

    UFUNCTION(server, reliable, WithValidation)
    void Server_SetTable(ATable* NewTable);
    UFUNCTION(server, reliable, WithValidation)
    void Server_SetState(FBilliardistState NewState);
};
