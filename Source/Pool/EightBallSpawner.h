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
};
