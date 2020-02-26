// Copyright 2019 Andrei Vikarchuk.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "BallInHandUpdateListener.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UBallInHandUpdateListener : public UInterface
{
    GENERATED_BODY()
};

/**
 *
 */
class POOL_API IBallInHandUpdateListener
{
    GENERATED_BODY()

    // Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
    virtual void OnBallInHandUpdate(class ABall* const NewBall) = 0;
};
