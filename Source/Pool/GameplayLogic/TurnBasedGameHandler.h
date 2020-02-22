// Copyright 2019 Andrei Vikarchuk.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TurnBasedGameHandler.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UTurnBasedGameHandler : public UInterface
{
    GENERATED_BODY()
};

class ITurnBasedPlayer;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTurnEnd);

/**
 *
 */
class POOL_API ITurnBasedGameHandler
{
    GENERATED_BODY()

    // Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
    virtual void EndCurrentTurn() = 0;
    virtual bool IsMyTurn(const ITurnBasedPlayer* Player) = 0;
};
