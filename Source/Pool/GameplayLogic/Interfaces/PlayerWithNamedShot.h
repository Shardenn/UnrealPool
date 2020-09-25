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

/**
 * An interface to be inherited by PlayerState which supports named shot rule
 */
class POOL_API IPlayerWithNamedShot
{
    GENERATED_BODY()

public:
    virtual void NameShot(UPocketArea* SelectedPocket) = 0;
};
