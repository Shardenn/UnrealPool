// Copyright 2019 Andrei Vikarchuk.

#pragma once

#include "CoreMinimal.h"
#include "GameplayLogic/PoolGameState.h"
#include "GameplayLogic/Interfaces/GameWithHandableBall.h"
#include "EightBallGameState.generated.h"

/**
 *
 */
UCLASS()
class POOL_API AEightBallGameState : public APoolGameState, public IGameWithHandableBall
{
    GENERATED_BODY()

public:
    class ABallAmerican* const GetCueBall();

    virtual void OnFrameRestarted() override;

    virtual void GiveBallInHand(const TScriptInterface<IPlayerWithHandableBall>& Player, ABall* const Ball) override;
    virtual void TakeBallFromHand(const TScriptInterface<IPlayerWithHandableBall>& Player, ABall* Ball) override;
protected:
    virtual void HandleTurnEnd_Internal() override;
    virtual bool DecideWinCondition() override;
    virtual void HandlePocketedBall(class ABall* Ball) override;

    void HandleBlackBallOutOfPlay();

    UPROPERTY(Replicated)
    class ABallAmerican* CueBall{ nullptr };

    TScriptInterface<class IPlayerWithHandableBall> PlayerWithCueBall{ nullptr };

    virtual void GiveBallInHand_Internal(const TScriptInterface<IPlayerWithHandableBall>& Player, ABall* const Ball) override;
    virtual void TakeBallFromHand_Internal(const TScriptInterface<IPlayerWithHandableBall>& Player, ABall* Ball) override;
private:
    bool FindAndInitializeCueBall();
    
    UFUNCTION(Server, Reliable, WithValidation)
    void Server_GiveBallInHand(const TScriptInterface<IPlayerWithHandableBall>& Player, ABall* Ball);

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_TakeBallFromHand(const TScriptInterface<IPlayerWithHandableBall>& Player, ABall* Ball);
};
