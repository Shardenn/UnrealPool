// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "PoolGameMode.generated.h"

/**
 * 
 */
UCLASS()
class POOL_API APoolGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
    /**
    * Overrides GameModeBase restartPlayer.
    * Spawns player on table spline so that he looks at the table.
    */
    virtual void RestartPlayer(AController* Controller) override;
    virtual bool ReadyToStartMatch_Implementation() override;
    virtual void PostLogin(APlayerController* NewPlayer) override;

    bool InitializeTable();

protected:
    class ATable* GameTable = nullptr;
    TArray<AController*> PlayerControllers;

private:
    // Returns location at pline of the table and 
    // "look at table" rotation
    FTransform GetSpawnTransform();
};
