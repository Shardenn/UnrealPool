// Copyright 2019 Andrei Vikarchuk.

#pragma once

#include "CoreMinimal.h"
#include "Objects/Table/BallSpawner.h"
#include "EightBallSpawner.generated.h"

/**
 * 
 */
UCLASS()
class POOL_API UEightBallSpawner : public UBallSpawner
{
	GENERATED_BODY()
	
public:
    UEightBallSpawner();
    virtual TArray<class ABall*> Spawn() override;

private:
    // returns random ball number, available from the param.
    // Optionally removes the number from the array afterwards
    uint8 GetRandomBallNum(TArray<uint8>& AvailableNumbers, bool bRemoveNumber = false);
};
