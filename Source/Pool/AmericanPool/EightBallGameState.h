// Copyright 2019 Andrei Vikarchuk.

#pragma once

#include "CoreMinimal.h"
#include "GameplayLogic/PoolGameState.h"
#include "GameplayLogic/Interfaces/GameWithHandableBall.h"
#include "GameplayLogic/Interfaces/GameWithMainCueBall.h"
#include "GameplayLogic/Interfaces/GSWithNamedShot.h"
#include "EightBallGameState.generated.h"

class UPocketArea;
class ABall;

/**
 *
 */
UCLASS()
class POOL_API AEightBallGameState : public APoolGameState, public IGameWithHandableBall, public IGameWithMainCueBall, public IGSWithNamedShot
{
    GENERATED_BODY()

public:
    class ABall* GetCueBall() override;

    virtual void OnFrameRestarted() override;

    virtual void GiveBallInHand(const TScriptInterface<IPlayerWithHandableBall>& Player, ABall* const Ball) override;
    virtual void TakeBallFromHand(const TScriptInterface<IPlayerWithHandableBall>& Player, ABall* Ball) override;

    virtual void RegisterNamedShot(UPocketArea* SelectedPocket, ABall* SelectedBall) override;
protected:
    virtual void HandleTurnEnd_Internal() override;
    virtual bool DecideWinCondition() override;
    virtual void HandlePocketedBall(class ABall* Ball) override;

    void HandleBlackBallOutOfPlay();

    UPROPERTY(Replicated)
    class ABall* CueBall{ nullptr };

    FNamedShot RegisteredNamedShot;
    UPROPERTY(BlueprintAssignable)
    FOnNamedShotRegistered OnNamedShotRegistered;
    
    TScriptInterface<class IPlayerWithHandableBall> PlayerWithCueBall{ nullptr };

    virtual void GiveBallInHand_Internal(const TScriptInterface<IPlayerWithHandableBall>& Player, ABall* const Ball) override;
    virtual void TakeBallFromHand_Internal(const TScriptInterface<IPlayerWithHandableBall>& Player, ABall* Ball) override;
    
    virtual void RegisterNamedShot_Internal(UPocketArea* SelectedPocket, ABall* SelectedBall) override;
private:
    bool FindAndInitializeCueBall();
    
    UFUNCTION(Server, Reliable, WithValidation)
    void Server_GiveBallInHand(const TScriptInterface<IPlayerWithHandableBall>& Player, ABall* Ball);

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_TakeBallFromHand(const TScriptInterface<IPlayerWithHandableBall>& Player, ABall* Ball);

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_RegisterNamedShot(UPocketArea* SelectedPocket, ABall* SelectedBall);

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_BroadcastNamedShotRegistered(FNamedShot NamedShot);
};
