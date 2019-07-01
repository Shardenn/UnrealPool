// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Ball.h"
#include "PoolGameModeBase.h"
#include "BilliardistController.generated.h"

class ACameraManager;
class ABilliardist;
class USplineComponent;

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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerStateChange, FBilliardistState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSelectedBallUpdated, ABall*, NewBall);

/**
 * 
 */
UCLASS()
class POOL_API ABilliardistController : public APlayerController
{
    GENERATED_BODY()

public:
    ABilliardistController();
    virtual void Tick(float DeltaTime) override;
    
    virtual void SetupInputComponent() override;

    void LookAtBall();

    UPROPERTY(BlueprintAssignable)
    FOnPlayerStateChange OnStateChange;
    UPROPERTY(BlueprintAssignable)
    FOnSelectedBallUpdated OnSelectedBallUpdate;

    UFUNCTION(BlueprintCallable, Category = "Billiardist Character", meta = (DisplayName = "Set State"))
    void SetState(FBilliardistState NewState);

    UFUNCTION(BlueprintPure, Category = "Billiardist Character", meta = (DisplayName = "Get State"))
    FBilliardistState GetState() { return m_eState; }

    UFUNCTION(BlueprintPure, Category = "Billiardist Character", meta = (DisplayName = "Get Hit Strength Alpha"))
    float GetHitStrengthAlpha() { return m_fHitStrengthAlpha; }

    UFUNCTION(BlueprintCallable, Category = "Billiardist controller", meta = (DisplayName = "Get stored billiardist"))
    ABilliardist* GetBilliardist()                           { return m_pControlledBilliardist; }

    UFUNCTION(BlueprintCallable, Category = "Billiardist controller", meta = (DisplayName = "Initialize billiardist controller"))
    void Initialize(ATable* Table, ABilliardist* BillPawn, ACameraManager* CamMan);

    UFUNCTION(BlueprintCallable, Category = "Billiardist controller", meta = (DisplayName = "Set selected ball"))
    void SetBall(ABall* NewBall);
    UFUNCTION(BlueprintCallable, Category = "Billiardist controller", meta = (DisplayName = "Set billiardist pawn"))
    void SetBilliardist(ABilliardist* BillPawn);
    UFUNCTION(BlueprintCallable, Category = "Billiardist controller", meta = (DisplayName = "Set camera manager"))
    void SetCameraManager(ACameraManager* CamMan);

    // sets the new table for controller to know what spline is used by the player
    UFUNCTION(BlueprintCallable, Category = "Billiardist controller", meta = (DisplayName = "Set table and spline"))
    void SetTable(ATable* NewTable);

    UFUNCTION(BlueprintCallable, Category = "Billiardist controller", meta = (DisplayName = "Try raycast ball"))
    bool TryRaycastBall(ABall*& Ball);

    UFUNCTION(BlueprintCallable, Category = "Billiardist controller", meta = (DisplayName = "Switch pawn"))
    void SwitchPawn(APawn* newPawn);
protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Billiardist controller | Camera management", meta = (DisplayName = "Lock outgoing camera"))
    bool m_bLockOutgoing{ false };
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Billiardist controller | Camera management", meta = (DisplayName = "Default camera blend time"))
    float m_fCameraBlendTime { 0.5f };
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Billiardist controller | Camera management", meta = (DisplayName = "Aiming pawn pitch min"))
    float m_fAimingPitchMin = -10.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Billiardist controller | Camera management", meta = (DisplayName = "Current observing camera"))
    int32 m_dCameraNumber = 0;

    float m_fDistanceAlongSpline{ 0.0f };

    UPROPERTY(EditAnywhere, Category = "Billiardist controller | RayCasting to a ball", meta = (DisplayName = "Crosshair X location"))
    float m_fCrosshairXLocation{ 0.5f };
    UPROPERTY(EditAnywhere, Category = "Billiardist controller | RayCasting to a ball", meta = (DisplayName = "Crosshair Y location"))
    float m_fCrosshairYLocation{ 0.5f };
    UPROPERTY(EditAnywhere, Category = "Billiardist controller | RayCasting to a ball", meta = (DisplayName = "Ball search raycast length"))
    float m_fRaycastLength{ 200.f };

    UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Billiardist controller | Gameplay process", meta = (DisplayName = "Player spline")) // needs to be replicated for movement along spline
    USplineComponent* m_pPlayerSpline = nullptr;

    UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Billiardist controller | Gameplay process", meta = (DisplayName = "Selected ball"))
    ABall* m_pSelectedBall { nullptr };    

    UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Billiardist controller | Gameplay process", meta = (DisplayName = "Camera manager"))
    ACameraManager* m_pCameraManager;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Billiardist controller | Gameplay process", meta = (DisplayName = "MIN hit strength"))
    float m_fHitStrengthMin = 50.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Billiardist controller | Gameplay process", meta = (DisplayName = "MAX hit strength"))
    float m_fHitStrengthMax = 500.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Billiardist controller | Gameplay process", meta = (DisplayName = "Hit strength change speed"))
    float m_fHitStrengthChangeSpeed = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Billiardist controller | Gameplay process", meta = (DisplayName = "High hit strength chenge speed"))
    float m_fHitStrengthChangeHigh = 2.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Billiardist controller | Gameplay process", meta = (DisplayName = "Hit strength alpha from 0 to 1"))
    float m_fHitStrengthAlpha = 0.f; // from 0 to 1
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Billiardist controller | Gameplay process", meta = (DisplayName = "Current hit strength"))
    float m_fCurrentHitStrength = m_fHitStrengthMin;

    UFUNCTION(BlueprintImplementableEvent, Category = "Billiardist controller", meta = (DisplayName = "On Player State Changed"))
    void OnPlayerStateChangedEvent(FBilliardistState NewState);

    virtual void BeginPlay() override;
private:
    UFUNCTION(reliable, server, WithValidation)
    void Server_SwitchPawn(APawn* newPawn);

    UFUNCTION(reliable, server, WithValidation)
    void Server_MovePlayer(FVector NewLocation);
    UFUNCTION(reliable, NetMulticast, WithValidation)
    void Multicast_MovePlayer(FVector NewLocation);
    
    

    UFUNCTION(reliable, server, WithValidation)
    void Server_Initialize(ATable* Table, ABilliardist* BillPawn, ACameraManager* CamMan);

    UFUNCTION(reliable, server, WithValidation)
    void Server_SetBall(ABall* NewBall);
    UFUNCTION(reliable, server, WithValidation)
    void Server_SetTable(ATable* NewTable);
    UFUNCTION(reliable, server, WithValidation)
    void Server_SetBilliardist(ABilliardist* NewBill);
    UFUNCTION(reliable, server, WithValidation)
    void Server_SetCameraManager(ACameraManager* CamMan);

    UFUNCTION(server, reliable, WithValidation)
    void Server_SetState(FBilliardistState NewState);
    UPROPERTY(Replicated)
    FBilliardistState m_eState         { FBilliardistState::WALKING };
    UPROPERTY(Replicated)
    FBilliardistState m_ePreviousState { FBilliardistState::WALKING };

    UPROPERTY(Replicated)
    ABilliardist* m_pControlledBilliardist{ nullptr };

    bool m_bStrengthIncreasing = true;

    FVector Direction{ FVector::ZeroVector }; // direction that the pawn would go in case we do not have a spline path

    bool GetLookDirection(FVector2D ScreenLocation, FVector & LookDirection) const;

    void SetExaminingView();
    void ReturnFromExaminingView();

    UFUNCTION()
    void MoveForward(float Value);
    UFUNCTION()
    void MoveRight(float Value);
    UFUNCTION()
    void ActionPressHandle();
    UFUNCTION()
    void ReturnPressHandle();
    UFUNCTION()
    void ExaminingPressHandle();
};
