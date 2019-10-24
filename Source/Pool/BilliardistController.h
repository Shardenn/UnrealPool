// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BilliardistController.generated.h"

class ABilliardistPawn;

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

    UFUNCTION(BlueprintCallable, Category = "Billiardist controller")
    bool TryRaycastBall(class ABall*& Ball);

    UFUNCTION(BlueprintCallable, Category = "Billiardist controller")
    bool TryRaycastTable(FVector& RaycastHit);

    UFUNCTION(BlueprintCallable, Category = "Billiardist controller")
    void SubscribeToPlayerStateChange(ABilliardistPawn* Billardist);

    void HandleMatchEnd();
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
private:
    UFUNCTION(reliable, server, WithValidation)
    void Server_SubscribeToStateChange(ABilliardistPawn* Billardist);

    bool GetLookDirection(FVector2D ScreenLocation, FVector & LookDirection) const;

    void SetExaminingView();
    void ReturnFromExaminingView();
};
