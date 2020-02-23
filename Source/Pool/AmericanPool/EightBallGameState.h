// Copyright 2019 Andrei Vikarchuk.

#pragma once

#include "CoreMinimal.h"
#include "GameplayLogic/PoolGameState.h"
#include "EightBallGameState.generated.h"

/**
 *
 */
UCLASS()
class POOL_API AEightBallGameState : public APoolGameState
{
    GENERATED_BODY()

public:
    class ABallAmerican* const GetCueBall();

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_GiveBallInHand(APoolPlayerState* PlayerState = nullptr);

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_TakeBallFromHand();

    virtual void OnFrameRestarted() override;
protected:
    virtual void HandleTurnEnd_Internal() override;
    virtual bool DecideWinCondition() override;
    virtual void HandlePocketedBall(class ABall* Ball) override;

    void HandleBlackBallOutOfPlay();

    UPROPERTY(Replicated)
    class ABallAmerican* CueBall = nullptr;

private:
    bool FindAndInitializeCueBall();
};
