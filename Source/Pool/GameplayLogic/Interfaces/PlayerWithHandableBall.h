// Copyright 2019 Andrei Vikarchuk.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PlayerWithHandableBall.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UPlayerWithHandableBall : public UInterface
{
    GENERATED_BODY()
};

class ABall;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBallInHandUpdate, ABall* const, Ball);

/**
 *
 */
class POOL_API IPlayerWithHandableBall
{
    GENERATED_BODY()

        // Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
    virtual void SetBallInHand(ABall* const Ball, bool bInitialPlacement = false) = 0;
    virtual bool GetIsBallInHand() const = 0;
    virtual void PlaceHandedBall(const FVector& TablePoint) = 0;
    virtual ABall* GetHandedBall() const noexcept = 0;
    virtual void SubscribeToBallInHandUpdate(const TScriptInterface<class IBallInHandUpdateListener>&) = 0;
protected:
    // For server chain of call
    virtual void PlaceHandedBall_Internal(const FVector& TablePoint) = 0;
};
