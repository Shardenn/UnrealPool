// Copyright 2019 Andrei Vikarchuk.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PlayerWithNamedShot.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UPlayerWithNamedShot : public UInterface
{
    GENERATED_BODY()
};

class UPocketArea;
class ABall;

/**
 * An interface to be inherited by PlayerState which supports named shot rule
 */
class POOL_API IPlayerWithNamedShot
{
    GENERATED_BODY()

public:
    virtual void NameShot(UPocketArea* SelectedPocket, ABall* SelectedBall) = 0;
    virtual void PredictShot(UPocketArea* SelectedPocket, ABall* SelectedBall) = 0;
protected:
    virtual void NameShot_Internal(UPocketArea* SelectedPocket, ABall* SelectedBall) = 0;
    virtual void PredictShot_Internal(UPocketArea* SelectedPocket, ABall* SelectedBall) = 0;
};
