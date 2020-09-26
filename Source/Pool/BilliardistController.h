// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayLogic/Interfaces/NetworkTimeProvider.h"
#include "Player/Billiardist/BilliardistStates.h"
#include "BilliardistController.generated.h"

class ABilliardistPawn;
class UPocketArea;

/**
 * 
 */
UCLASS()
class POOL_API ABilliardistController : public APlayerController, public INetworkTimeProvider
{
    GENERATED_BODY()

public:
    ABilliardistController();
    virtual void Tick(float DeltaTime) override;
    
    void LookAtBall();

    UFUNCTION(BlueprintCallable, Category = "Billiardist controller")
    class ABall* TryRaycastBall();

    UFUNCTION(BlueprintCallable, Category = "Billiardist controller")
    bool TryRaycastTable(FVector& RaycastHit);

    UFUNCTION(BlueprintCallable, Category = "Billiardist controller")
    UPocketArea* TryRaycastPocketArea();

    UFUNCTION(BlueprintCallable, Category = "Billiardist controller")
    void SubscribeToPlayerStateChange(ABilliardistPawn* Billardist);

    void HandleMatchEnd();

    virtual int64 GetNetworkTime() override;
    virtual int64 GetTimeOffsetFromServer() override { return TimeOffsetFromServer; }
    virtual bool IsTimeOffsetValid() override { return bTimeOffsetFromServerValid; }
    static int64 GetLocalTime();
protected:
    virtual void BeginPlay() override;

    UFUNCTION(BlueprintImplementableEvent, Category = "Billiardist controller", Meta = (DisplayName = "On Player State Changed"))
    void OnPlayerStateChangedEvent(FBilliardistState NewState);

    UPROPERTY(EditAnywhere, Category = "Billiardist controller | RayCasting to a ball", meta = (DisplayName = "Crosshair X location"))
    float CrosshairXLocation{ 0.5f };
    UPROPERTY(EditAnywhere, Category = "Billiardist controller | RayCasting to a ball", meta = (DisplayName = "Crosshair Y location"))
    float CrosshairYLocation{ 0.5f };
    UPROPERTY(EditAnywhere, Category = "Billiardist controller | RayCasting to a ball", meta = (DisplayName = "Ball search raycast length"))
    float RaycastLength{ 200.f };

    UFUNCTION()
    void OnPlayerStateChanged(FBilliardistState newState);

    UFUNCTION(BlueprintImplementableEvent)
    void OnMatchEnd();



    int64 TimeOffsetFromServer{ 0 };
    int64 ServerTimeRequestWasPlaced{ 0 };
    bool bTimeOffsetFromServerValid{ false };
private:
    UFUNCTION(reliable, server, WithValidation)
    void Server_SubscribeToStateChange(ABilliardistPawn* Billardist);

    bool GetLookDirection(FVector2D ScreenLocation, FVector & LookDirection) const;

    void SetExaminingView();
    void ReturnFromExaminingView();

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_GetServerTime();

    UFUNCTION(Client, Reliable)
    void Client_GetServerTime(int64 ServerTime);
};
