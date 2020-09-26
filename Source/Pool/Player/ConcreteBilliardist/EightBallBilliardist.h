// Copyright 2019 Andrei Vikarchuk.

#pragma once

#include "CoreMinimal.h"
#include "Player/Billiardist/BilliardistPawnWithPlacableBall.h"
#include "Player/Interfaces/BilliardistWithNamedShot.h"
#include "EightBallBilliardist.generated.h"

/**
 *
 */
UCLASS()
class POOL_API AEightBallBilliardist : public ABilliardistPawnWithPlacableBall, public IBilliardistWithNamedShot
{
    GENERATED_BODY()

protected:
    virtual void NameShot(UPocketArea* SelectedPocket, ABall* SelectedBall) override;

    virtual void ActionReleaseHandle() override;

    UPocketArea* SelectedPocket{ nullptr };
};
