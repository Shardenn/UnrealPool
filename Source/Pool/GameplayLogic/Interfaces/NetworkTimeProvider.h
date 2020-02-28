// Copyright 2019 Andrei Vikarchuk.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "NetworkTimeProvider.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UNetworkTimeProvider : public UInterface
{
    GENERATED_BODY()
};

/**
 *
 */
class POOL_API INetworkTimeProvider
{
    GENERATED_BODY()

    // Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
    virtual int64 GetNetworkTime() = 0;
    virtual int64 GetTimeOffsetFromServer() = 0;
    virtual bool IsTimeOffsetValid() = 0;

protected:
};
