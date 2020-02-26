// Copyright 2019 Andrei Vikarchuk.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameWithHandableBall.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UGameWithHandableBall : public UInterface
{
    GENERATED_BODY()
};

class IPlayerWithHandableBall;
class ABall;

/**
 *
 */
class POOL_API IGameWithHandableBall
{
    GENERATED_BODY()

    // Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
    virtual void GiveBallInHand(const TScriptInterface<IPlayerWithHandableBall>& Player, ABall* const Ball) = 0;
    virtual void TakeBallFromHand(const TScriptInterface<IPlayerWithHandableBall>&, ABall* Ball) = 0;

protected:
    virtual void GiveBallInHand_Internal(const TScriptInterface<IPlayerWithHandableBall>& Player, ABall* const Ball) = 0;
    virtual void TakeBallFromHand_Internal(const TScriptInterface<IPlayerWithHandableBall>&, ABall* Ball) = 0;
};
