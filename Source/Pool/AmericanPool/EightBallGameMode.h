// Copyright 2019 Andrei Vikarchuk.

#pragma once

#include "CoreMinimal.h"
#include "GameplayLogic/PoolGameMode.h"
#include "GameplayLogic/Interfaces/GMWithNamedShot.h"
#include "EightBallGameMode.generated.h"

/**
 *
 */
UCLASS()
class POOL_API AEightBallGameMode : public APoolGameMode, public IGMWithNamedShot
{
    GENERATED_BODY()

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bNamedShotRuleActive{ true };

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bFirstTouchShouldBeFriendlyRuleActive{ true };
};
