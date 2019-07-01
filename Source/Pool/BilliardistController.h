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
    
    void LookAtBall();

    UFUNCTION(BlueprintCallable, Category = "Billiardist controller", meta = (DisplayName = "Initialize billiardist controller"))
    void Initialize(ATable* Table, ABilliardist* BillPawn, ACameraManager* CamMan);

    UFUNCTION(BlueprintCallable, Category = "Billiardist controller", meta = (DisplayName = "Try raycast ball"))
    bool TryRaycastBall(ABall*& Ball);
protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, Category = "Billiardist controller | RayCasting to a ball", meta = (DisplayName = "Crosshair X location"))
    float CrosshairXLocation{ 0.5f };
    UPROPERTY(EditAnywhere, Category = "Billiardist controller | RayCasting to a ball", meta = (DisplayName = "Crosshair Y location"))
    float CrosshairYLocation{ 0.5f };
    UPROPERTY(EditAnywhere, Category = "Billiardist controller | RayCasting to a ball", meta = (DisplayName = "Ball search raycast length"))
    float RaycastLength{ 200.f };

    UFUNCTION()
    void OnPlayerStateChanged(FBilliardistState newState);
private:
    UFUNCTION(reliable, server, WithValidation)
    void Server_Initialize(ATable* Table, ABilliardist* BillPawn, ACameraManager* CamMan);

    UFUNCTION(reliable, server, WithValidation)
    void Server_SubscribeToStateChange();

    bool GetLookDirection(FVector2D ScreenLocation, FVector & LookDirection) const;

    void SetExaminingView();
    void ReturnFromExaminingView();
};
