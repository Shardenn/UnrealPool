// Copyright 2019 Andrei Vikarchuk.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "BilliardistWithPlacableBall.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UBilliardistWithPlacableBall : public UInterface
{
    GENERATED_BODY()
};

class IPlayerWithHandableBall;

/**
 *
 */
class POOL_API IBilliardistWithPlacableBall
{
    GENERATED_BODY()

    // Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
    virtual void TryPlaceBall(const TScriptInterface<IPlayerWithHandableBall>& Player) = 0;
protected:
    // No input as implementation may take anything into account, starting from
    // ball's location ending up with its radius or other features.
    virtual bool IsBallPlacementValid() = 0;


};
