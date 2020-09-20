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

    ABall* GetCueBall();

    void AssignBallType(const FBallType& Type) noexcept { AssignedBallType = Type; }
    
    UFUNCTION(BlueprintPure)
    FBallType GetAssignedBallType() const noexcept { return AssignedBallType; }

    virtual void SubscribeToBallInHandUpdate(const TScriptInterface<IBallInHandUpdateListener>& Listener) override;

protected:
    UPROPERTY(Replicated)
    FBallType AssignedBallType = FBallType::NotInitialized;

    UPROPERTY(Replicated)
    ABall* BallHanded{ nullptr };

    UPROPERTY(Replicated)
    ABall* CueBall{ nullptr };

    TArray<TScriptInterface<IBallInHandUpdateListener>> BallInHandUpdateListeners;

    virtual void SetIsMyTurn(const bool bInMyTurn) noexcept override;

    virtual void PlaceHandedBall_Internal(const FVector& TablePoint) override;
    virtual void OnFrameRestarted_Internal() override;
private:
    UFUNCTION(Server, Reliable, WithValidation)
    void Server_PlaceHandedBall(const FVector& TablePoint);

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_BroadcastBallInHandUpdate(ABall* Ball);
};
