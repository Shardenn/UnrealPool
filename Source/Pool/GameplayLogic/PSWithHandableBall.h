// Copyright 2019 Andrei Vikarchuk.

#pragma once

#include "CoreMinimal.h"
#include "GameplayLogic/PoolPlayerState.h"
#include "GameplayLogic/Interfaces/PlayerWithHandableBall.h"
#include "AmericanPool/EightPoolBallType.h"
#include "PSWithHandableBall.generated.h"

class ABall;
class UPocketArea;
class IBallInHandUpdateListener;

/**
 *
 */
UCLASS()
class POOL_API APSWithHandableBall : public APoolPlayerState, public IPlayerWithHandableBall
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnBallInHandUpdate OnBallInHandUpdate;

    virtual void SetBallInHand(ABall* CueBall, bool bInitialPlacement = false) override;
    virtual ABall* GetHandedBall() const noexcept override { return HandedBall.Ball; }
    virtual bool GetIsBallInHand() const override { return HandedBall.Ball != nullptr; }
    virtual void PlaceHandedBall(const FVector& Location) override;
    
    bool GetIsInitialPlacement() const noexcept { return HandedBall.bInitialPlacement; }
    
    virtual void SubscribeToBallInHandUpdate(const TScriptInterface<IBallInHandUpdateListener>& Listener) override;

    UPROPERTY(Replicated)
    FHandedBallState HandedBall;

protected:
    TArray<TScriptInterface<IBallInHandUpdateListener>> BallInHandUpdateListeners;

    virtual void SetIsMyTurn(const bool bInMyTurn) noexcept override;

    virtual void PlaceHandedBall_Internal(const FVector& Location) override;
private:
    UFUNCTION(Server, Reliable, WithValidation)
    void Server_PlaceHandedBall(const FVector& Location);

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_BroadcastBallInHandUpdate(ABall* Ball, bool bInitialPlacementIn);
};
