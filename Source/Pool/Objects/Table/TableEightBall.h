// Copyright 2019 Andrei Vikarchuk.

#pragma once

#include "CoreMinimal.h"
#include "Objects/Table/Table.h"
#include "TableEightBall.generated.h"

/**
 * 
 */
UCLASS()
class POOL_API ATableEightBall : public ATable
{
	GENERATED_BODY()
	
public:
    ATableEightBall();

    virtual void BeginPlay() override;
};
