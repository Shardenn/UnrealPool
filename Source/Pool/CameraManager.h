// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Camera/CameraActor.h"
#include "CameraManager.generated.h"

UENUM(BlueprintType)
enum class FCameraType : uint8
{
    TopDown         UMETA(DisplayName = "TopDown"), 
    Corner          UMETA(DisplayName = "Corner"),
    Tournament      UMETA(DisplayName = "Tournament"),
    Aiming          UMETA(DisplayName = "Aiming")
};

USTRUCT(BlueprintType)
struct FControlledCamera
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Billiardist Camera Manager", meta = (DisplayName = "Camera Actor"))
    ACameraActor* Camera = nullptr;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Billiardist Camera Manager", meta = (DisplayName = "Blend Time"))
    float fBlendTime = 0.0f;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Billiardist Camera Manager", meta = (DisplayName = "Lock Outgoing?"))
    bool bLockOutgoing = false;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Billiardist Camera Manager", meta = (DisplayName = "Camera Type"))
    FCameraType eCameraType = FCameraType::TopDown;
};

UCLASS()
class POOL_API ACameraManager : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    ACameraManager();

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Possible controlled cameras"))
    TArray<FControlledCamera> ControlledCameras;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Aiming Camera | Camera",
        meta = (DisplayName = "Aiming pawn with camera"))
    APawn* AimingPawn = nullptr;

    
protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    
public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

};
