// Copyright 2019 Andrei Vikarchuk.

#pragma once

#include "CoreMinimal.h"
#include "Objects/Ball.h"
#include "BallAmerican.generated.h"

/**
 *
 */
UENUM(BlueprintType)
enum class FBallType : uint8
{
    Stripe,
    Solid,
    Black,
    Cue,
    NotInitialized
};

UCLASS()
class POOL_API ABallAmerican : public ABall
{
    GENERATED_BODY()

public:
    ABallAmerican();
    ABallAmerican(uint8 BallNum);
    FBallType BallType = FBallType::NotInitialized;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 BallNumber = 1;
};
