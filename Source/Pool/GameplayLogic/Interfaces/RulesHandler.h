// Copyright 2019 Andrei Vikarchuk.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "RulesHandler.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class URulesHandler : public UInterface
{
    GENERATED_BODY()
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerFouled);

/**
 *
 */
class POOL_API IRulesHandler
{
    GENERATED_BODY()

    // Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
    virtual void HandleTurnEnd() = 0;
    virtual void AssignFoul() = 0;
protected:
    // Override THIS if you want to define turn end implementation
    virtual void HandleTurnEnd_Internal() = 0;
    // Override THIS if you want to define fould assigning implementation
    virtual void AssignFoul_Internal() = 0;
};
