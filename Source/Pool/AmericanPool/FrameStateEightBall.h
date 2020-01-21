// Copyright 2019 Andrei Vikarchuk.

#pragma once

#include "CoreMinimal.h"
#include "GameplayLogic/FrameState.h"
#include "FrameStateEightBall.generated.h"

/**
 *
 */
UCLASS()
class POOL_API UFrameStateEightBall : public UFrameState
{
    GENERATED_BODY()

public:
    void SetCueBall(class ABallAmerican* InBall) { CueBall = InBall; }
    class ABallAmerican* GetCueBall() const { return CueBall; }
protected:
    class ABallAmerican* CueBall = nullptr;

    virtual void Server_HandleTurnEnd() override;
    void HandleBlackBallOutOfPlay();
    bool DecideWinCondition();
    void AssignFoul() {};
};
