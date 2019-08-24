// Copyright 2019 Andrei Vikarchuk.

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

    uint8 GetRequiredBallsToPocket() { return RequiredBallsToPocket; }

    bool InitializeTable();

    class USplineComponent* GetSpline();

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
    uint8 RequiredPlayersReadyNum = 2;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
    uint8 RequiredFramesToWin = 2;

protected:
    class ATable* GameTable = nullptr;
    TArray<ABilliardistController*> PlayerControllers;

    // number of balls we are required to pocket before the 8 ball
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
    uint8 RequiredBallsToPocket = 7;
private:
    // Returns location at pline of the table and 
    // "look at table" rotation
    FTransform GetSpawnTransform();
};
