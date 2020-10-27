// Copyright 2019 Andrei Vikarchuk.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ReplayController.generated.h"

/**
 * 
 */
UCLASS()
class POOL_API AReplayController : public APlayerController
{
	GENERATED_BODY()

public:
	AReplayController(const FObjectInitializer& Initializer);

	UFUNCTION(BlueprintCallable, Category = "CurrentReplay")
	bool SetCurrentReplayPauseState(bool bDoPause);
	UFUNCTION(BlueprintCallable, Category = "CurrentReplay")
	float GetCurrentReplayTotalTimeInSec() const;
	UFUNCTION(BlueprintCallable, Category = "CurrentReplay")
	float GetCurrentReplayCurrentTimeInSec() const;
	UFUNCTION(BlueprintCallable, Category = "CurrentReplay")
	void SetCurrentReplayCurrentTimeToSec(float Seconds);
	UFUNCTION(BlueprintCallable, Category = "CurrentReplay")
	void SetCurrentReplayPlayRate(float PlayRate = 1.f);
protected:
	// For saving anti-aliasing and motion blur settings
	int32 PreviousAASetting;
	int32 PreviousMBSetting;

	
};

