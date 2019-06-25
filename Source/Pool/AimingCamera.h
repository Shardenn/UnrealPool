// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "AimingCamera.generated.h"

class ABall;

UENUM(BlueprintType)
enum class FAimingCameraState : uint8
{
    GoingIn  UMETA(DisplayName = "GoingIn"),
    GoingOut UMETA(DisplayName = "GoingOut"),
    InPlayer UMETA(DisplayName = "InPlayer"),
    InBall   UMETA(DisplayName = "InBall")
};

UCLASS()
class POOL_API AAimingCamera : public APawn
{
    GENERATED_BODY()

public:
    // Sets default values for this pawn's properties
    AAimingCamera();

    // Called every frame
    virtual void Tick(float DeltaTime) override;

    void ReturnPressHandle();

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings",
        meta = (DisplayName = "Camera blend IN speed"))
    float BlendInSpeed = 3.f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings",
        meta = (DisplayName = "Camera blend OUT speed"))
    float BlendOutSpeed = 3.f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings",
        meta = (DisplayName = "Lock outgoing?"))
    bool bLockOutgoing = false;

    UFUNCTION(BlueprintCallable, Category = "Aiming camera", meta = (DisplayName = "Set State"))
    void SetState(FAimingCameraState newState);

    UFUNCTION(BlueprintCallable, Category = "Aiming camera", meta = (DisplayName = "Get State"))
    FAimingCameraState GetState() { return m_eState; }

    UFUNCTION(BlueprintCallable, Category = "Aiming camera", meta = (DisplayName = "Set Ball"))
    void SetBall(ABall* newBall) { m_pSelectedBall = newBall; }

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gameplay", 
        meta = (DisplayName = "Current state"))
    FAimingCameraState m_eState = FAimingCameraState::InPlayer;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gameplay",
        meta = (DisplayName = "Selected ball"))
    ABall* m_pSelectedBall = nullptr;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gameplay",
        meta = (DisplayName = "Spring arm length"))
    float m_fSpringArmLength = 40.f;
private:
    float m_fAlpha = 0.0f; // for interpolation for movement
    FVector m_vStartingLocation;
    FVector m_vFinishLocation;
    FVector m_vLastPlayerLocation;
};
