// Copyright 2019 Andrei Vikarchuk.

#pragma once

#include "CoreMinimal.h"
#include "Player/Billiardist/BilliardistPawnWithPlacableBall.h"
#include "EightBallBilliardist.generated.h"

/**
 *
 */
UCLASS()
class POOL_API AEightBallBilliardist : public ABilliardistPawnWithPlacableBall
{
    GENERATED_BODY()

protected:
    virtual void ActionPressHandle() override;
    virtual void ActionReleaseHandle() override;
    virtual void ReturnPressHandle() override;
};
