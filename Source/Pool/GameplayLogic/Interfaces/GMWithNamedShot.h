// Copyright 2019 Andrei Vikarchuk.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GMWithNamedShot.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UGMWithNamedShot : public UInterface
{
	GENERATED_BODY()
};

/**
 * An interface to be inherited by GameMode which supports a named shot rule
 */
class POOL_API IGMWithNamedShot
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
};
