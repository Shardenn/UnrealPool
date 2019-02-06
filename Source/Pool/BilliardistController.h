// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Billiardist.h"
#include "Ball.h"
#include "PoolGameModeBase.h"
#include "BilliardistController.generated.h"

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

    UFUNCTION(BlueprintCallable, meta = (DisplayName = "Initialize Billiardist Pawn"))
    void InitializeBilliardistPawn();

    UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set Selected Ball"))
    void SetBall(ABall* NewBall);

    // sets the new table for controller to know what spline is used by the player
    UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set Table And Spline"))
    void SetTable(ATable* NewTable);
protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "Lock outgoing camera"))
    bool m_bLockOutgoing{ false };
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "Cameras blend time"))
    float m_fCameraBlendTime { 0.5f };

    float m_fDistanceAlongSpline{ 0.0f };
    UPROPERTY(Replicated) // needs to be replicated for movement along spline
    USplineComponent* m_pPlayerSpline{ nullptr };
    UPROPERTY(Replicated)
    ABall* m_pSelectedBall { nullptr };    

    virtual void BeginPlay() override;
private:
    UFUNCTION(reliable, server, WithValidation)
    void Server_MovePlayer(FVector NewLocation);
    UFUNCTION(reliable, NetMulticast, WithValidation)
    void Multicast_MovePlayer(FVector NewLocation);
    
    UFUNCTION(reliable, server, WithValidation)
    void Server_SetBall(ABall* NewBall);

    UFUNCTION(reliable, server, WithValidation)
    void Server_SetTable(ATable* NewTable);

    UFUNCTION()
    void OnPlayerStateChanged(FBilliardistState NewState);
    UFUNCTION(reliable, server, WithValidation)
    void Server_SubscribeToStateChange();

    ABilliardist* m_pControlledBilliardist{ nullptr };

    FVector Direction{ FVector::ZeroVector }; // direction that the pawn would go in case we do not have a spline path
};
