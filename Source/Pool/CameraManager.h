// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Camera/CameraActor.h"
#include "CameraManager.generated.h"

UENUM(BlueprintType)
enum class FCameraType : uint8
{
    TopDown         UMETA(DisplayName = "TopDown"),     // just walking around the table, examining
    Corner          UMETA(DisplayName = "Corner"),     // if we are playing RU billiard, we can pick any ball for the shot
    Tournament      UMETA(DisplayName = "Tournament"),      // when a ball is picked, we aim for the shot, holding the cue near the ball
};

USTRUCT(BlueprintType)
struct FControlledCamera
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Camera Actor"))
    ACameraActor* Camera = nullptr;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Blend Time"))
    float fBlendTime = 0.0f;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Lock Outgoing?"))
    bool bLockOutgoing = false;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Camera Type"))
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
protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    
public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

};
