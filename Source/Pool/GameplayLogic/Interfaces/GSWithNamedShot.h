// Copyright 2019 Andrei Vikarchuk.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GSWithNamedShot.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UGSWithNamedShot : public UInterface
{
    GENERATED_BODY()
};

class UPocketArea;

/**
 * An interface to be inherited by GameState which supports a named shot rule
 */
class POOL_API IGSWithNamedShot
{
    GENERATED_BODY()

public:
    virtual void RegisterNamedShot(UPocketArea* SelectedPocket) = 0;
};
