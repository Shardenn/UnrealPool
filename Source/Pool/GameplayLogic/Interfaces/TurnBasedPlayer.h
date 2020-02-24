// Copyright 2019 Andrei Vikarchuk.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TurnBasedPlayer.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UTurnBasedPlayer : public UInterface
{
    GENERATED_BODY()
};

/**
 *
 */
class POOL_API ITurnBasedPlayer
{
    GENERATED_BODY()

    // Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
    virtual void SetIsMyTurn(const bool bMyTurn) noexcept = 0;
    virtual bool GetIsMyTurn() const noexcept = 0;
    virtual void OnTurnStarted() = 0;
    virtual void OnTurnEnded() = 0;
};
