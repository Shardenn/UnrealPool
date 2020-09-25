// Copyright 2019 Andrei Vikarchuk.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "BilliardistWithNamedShot.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UBilliardistWithNamedShot : public UInterface
{
    GENERATED_BODY()
};

class UPocketArea;

/**
 * An interface to be inherited by a Pawn who supports named shots
 */
class POOL_API IBilliardistWithNamedShot
{
    GENERATED_BODY()

public:
    virtual void NameShot(UPocketArea* SelectedPocket) = 0;
};
