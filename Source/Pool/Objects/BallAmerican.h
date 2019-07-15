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

    UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
    FBallType BallType = FBallType::NotInitialized;
    
    UPROPERTY(ReplicatedUsing=OnRep_BallNumber, EditAnywhere, BlueprintReadWrite)
    int32 BallNumber = 1;

    UFUNCTION(BlueprintImplementableEvent)
    void SetupMaterial();

protected:
    UFUNCTION()
    void OnRep_BallNumber();
};
