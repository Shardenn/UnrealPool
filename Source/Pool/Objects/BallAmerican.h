// Copyright 2019 Andrei Vikarchuk.

#pragma once

#include "CoreMinimal.h"
#include "Objects/Ball.h"
#include "AmericanPool/EightPoolBallType.h"
#include "BallAmerican.generated.h"

/**
 *
 */


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

    UFUNCTION(BlueprintPure)
    int32 GetNumber() { return BallNumber; }

    UFUNCTION(BlueprintPure)
    FBallType GetType() { return BallType; }

protected:
    virtual void BeginPlay() override;

    UPROPERTY(ReplicatedUsing=OnRep_BallType, EditAnywhere, BlueprintReadWrite)
    FBallType BallType = FBallType::NotInitialized;
    UFUNCTION()
    void OnRep_BallType();

    UPROPERTY(ReplicatedUsing=OnRep_BallNumber, EditAnywhere, BlueprintReadWrite)
    int32 BallNumber = 0;
    UFUNCTION()
    void OnRep_BallNumber();
};
