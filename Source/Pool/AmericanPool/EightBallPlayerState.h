// Copyright 2019 Andrei Vikarchuk.

#pragma once

#include "CoreMinimal.h"
#include "GameplayLogic/PoolPlayerState.h"
#include "GameplayLogic/Interfaces/PlayerWithHandableBall.h"
#include "EightPoolBallType.h"
#include "EightBallPlayerState.generated.h"

class ABall;
class IBallInHandUpdateListener;
/**
 *
 */
UCLASS()
class POOL_API AEightBallPlayerState : public APoolPlayerState, public IPlayerWithHandableBall
{
    GENERATED_BODY()

public:
    virtual void SetBallInHand(ABall* const CueBall) override;
    virtual ABall* GetHandedBall() const noexcept override { return BallHanded; }
    virtual bool GetIsBallInHand() const override { return BallHanded != nullptr; }
    virtual void PlaceHandedBall(const FVector& TablePoint) override;

    void AssignBallType(const FBallType& Type) noexcept { AssignedBallType = Type; }
    
    UFUNCTION(BlueprintGetter)
    FBallType GetAssignedBallType() const noexcept { return AssignedBallType; }

    virtual void SubscribeToBallInHandUpdate(const TScriptInterface<IBallInHandUpdateListener>& Listener) override;

protected:
    UPROPERTY(replicated)
    FBallType AssignedBallType = FBallType::NotInitialized;

    UPROPERTY(replicated)
    class ABall* BallHanded{ nullptr };

    TArray<TScriptInterface<IBallInHandUpdateListener>> BallInHandUpdateListeners;

    virtual void SetIsMyTurn(const bool bInMyTurn) noexcept override;

    virtual void PlaceHandedBall_Internal(const FVector& TablePoint) override;
private:
    UFUNCTION(Server, Reliable, WithValidation)
    void Server_PlaceHandedBall(const FVector& TablePoint);

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_BroadcastBallInHandUpdate(ABall* Ball);
};
