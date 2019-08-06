// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "PoolGameMode.generated.h"

class ABilliardistController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFrameRestart);

/**
 * 
 */
UCLASS()
class POOL_API APoolGameMode : public AGameMode
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnFrameRestart OnFrameRestart;
    
    /**
    * Overrides GameModeBase restartPlayer.
    * Spawns player on table spline so that he looks at the table.
    */
    virtual void RestartPlayer(AController* Controller) override;
    virtual bool ReadyToStartMatch_Implementation() override;
    virtual void PostLogin(APlayerController* NewPlayer) override;

    virtual void HandleMatchHasStarted() override;
    virtual void HandleMatchHasEnded() override;

    void RestartFrame();

    bool InitializeTable();

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
    uint8 RequiredPlayersReadyNum = 2;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
    uint8 RequiredFramesToWin = 2;

protected:
    class ATable* GameTable = nullptr;
    TArray<ABilliardistController*> PlayerControllers;

private:
    // Returns location at pline of the table and 
    // "look at table" rotation
    FTransform GetSpawnTransform();
};
