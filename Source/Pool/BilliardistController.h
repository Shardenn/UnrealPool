// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Billiardist.h"
#include "Ball.h"
#include "PoolGameModeBase.h"
#include "BilliardistController.generated.h"

class ACameraManager;

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
    
    UPROPERTY(BlueprintAssignable)
    FOnSelectedBallUpdated OnSelectedBallUpdate;

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

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Billiardist controller | Camera management", meta = (DisplayName = "Lock outgoing camera"))
    bool m_bLockOutgoing{ false };
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Billiardist controller | Camera management", meta = (DisplayName = "Default camera blend time"))
    float m_fCameraBlendTime { 0.5f };

    float m_fDistanceAlongSpline{ 0.0f };

    UPROPERTY(EditAnywhere, Category = "Billiardist controller | Casting to a ball", meta = (DisplayName = "Crosshair X location"))
    float m_fCrosshairXLocation{ 0.5f };
    UPROPERTY(EditAnywhere, Category = "Billiardist controller | Casting to a ball", meta = (DisplayName = "Crosshair Y location"))
    float m_fCrosshairYLocation{ 0.5f };
    UPROPERTY(EditAnywhere, Category = "Billiardist controller | Casting to a ball", meta = (DisplayName = "Ball search raycast length"))
    float m_fRaycastLength{ 200.f };

    UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Billiardist controller", meta = (DisplayName = "Player spline")) // needs to be replicated for movement along spline
    USplineComponent* m_pPlayerSpline{ nullptr };
    UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Billiardist controller", meta = (DisplayName = "Selected ball"))
    ABall* m_pSelectedBall { nullptr };    
    UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Billiardist controller", meta = (DisplayName = "Camera manager"))
    ACameraManager* m_pCameraManager; // TODO camera manager is not replicated : 
    // if controller tries to self-initialize it when it is not assigned,
    // then camera manager is set on server, but on client it is  null.

    UFUNCTION(BlueprintImplementableEvent, Category = "Billiardist controller", meta = (DisplayName = "On Player State Changed"))
    void OnPlayerStateChangedEvent(FBilliardistState NewState);

    virtual void BeginPlay() override;
private:
    UFUNCTION(reliable, server, WithValidation)
    void Server_MovePlayer(FVector NewLocation);
    UFUNCTION(reliable, NetMulticast, WithValidation)
    void Multicast_MovePlayer(FVector NewLocation);
    
    UFUNCTION(reliable, server, WithValidation)
    void Server_SetBall(ABall* NewBall);

    UFUNCTION(reliable, server, WithValidation)
    void Server_Initialize(ATable* Table, ABilliardist* BillPawn, ACameraManager* CamMan);
    
    UFUNCTION(reliable, server, WithValidation)
    void Server_SetTable(ATable* NewTable);
    UFUNCTION(reliable, server, WithValidation)
    void Server_SetBilliardist(ABilliardist* NewBill);
    UFUNCTION(reliable, server, WithValidation)
    void Server_SetCameraManager(ACameraManager* CamMan);

    // need to be run on the client as this function handles camera
    UFUNCTION()
    void OnPlayerStateChanged(FBilliardistState NewState);
    UFUNCTION(Client, reliable)
    void Client_OnPlayerStateChanged(FBilliardistState NewState);

    UFUNCTION(reliable, server, WithValidation)
    void Server_SubscribeToStateChange();

    UPROPERTY(Replicated)
    ABilliardist* m_pControlledBilliardist{ nullptr };

    FVector Direction{ FVector::ZeroVector }; // direction that the pawn would go in case we do not have a spline path

    bool GetLookDirection(FVector2D ScreenLocation, FVector & LookDirection) const;
};
