// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Billiardist.h"
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

    UFUNCTION(BlueprintCallable, meta = (DisplayName = "Self Initialize Pawn"))
    void SelfInitializePawn();
protected:
    float m_fDistanceAlongSpline{ 0.0f };
    UPROPERTY(Replicated) // needs to be replicated for movement along spline
    USplineComponent* m_pPlayerSpline{ nullptr };
    virtual void BeginPlay() override;
private:
    UFUNCTION(reliable, server, WithValidation)
    void Server_MovePlayer(FVector NewLocation);
    UFUNCTION(reliable, NetMulticast, WithValidation)
    void Multicast_MovePlayer(FVector NewLocation);

    ABilliardist* m_pControlledBilliardist{ nullptr };

    FVector Direction{ FVector::ZeroVector }; // direction that the pawn would go in case we do not have a spline path
};
