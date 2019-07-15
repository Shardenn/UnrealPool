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

    UFUNCTION(Server, Reliable, WithValidation)
    void SetBallNumber(uint8 Number);

    UFUNCTION(Server, Reliable, WithValidation)
    void SetBallType(FBallType Type);

    UFUNCTION(BlueprintImplementableEvent)
    void SetupColorAndNumber();

    UFUNCTION(BlueprintImplementableEvent)
    void SetupStripeness();

protected:
    UPROPERTY(ReplicatedUsing=OnRep_BallType, EditAnywhere, BlueprintReadWrite)
    FBallType BallType = FBallType::NotInitialized;
    UFUNCTION()
    void OnRep_BallType();

    UPROPERTY(ReplicatedUsing=OnRep_BallNumber, EditAnywhere, BlueprintReadWrite)
    int32 BallNumber = 0;
    UFUNCTION()
    void OnRep_BallNumber();
};
