// Copyright 2019 Andrei Vikarchuk.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PlayerWithMainCueBall.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UPlayerWithMainCueBall : public UInterface
{
    GENERATED_BODY()
};

class ABall;

/**
 *
 */
class POOL_API IPlayerWithMainCueBall
{
    GENERATED_BODY()

        // Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
    virtual ABall* GetCueBall() = 0;
};
