// Copyright 2019 Andrei Vikarchuk.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Pool.h"
#include "BallsManager.generated.h"

class ABall;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBallPlayedOut, ABall*, Ball);
/**
 *
 */

UCLASS()
class POOL_API UBallsManager : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnBallPlayedOut OnBallPlayedOut;

    virtual bool IsSupportedForNetworking() const override
    {
        return true;
    }

    // Clears all arrays
    void Reset();

    // they are attached to awake and sleep events in ABall class.
    // NOTE And they are attached only on the server.
    // So the function is only called on server.
    // Only works if balls are correctly processed on server
    // (like LaunchBall is called as server function)
    void AddMovingBall(class ABall* Ball);
    void RemoveMovingBall(class ABall* Ball);

    void AddPocketedBall(class ABall* Ball);

    void AddDroppedBall(class ABall* Ball);
    /*
    void OnCueBallHit(UPrimitiveComponent* HitComponent,
            AActor* OtherActor,
            UPrimitiveComponent* OtherComp,
            FVector NormalImpulse,
            const FHitResult& Hit);
            */

    UFUNCTION(BlueprintPure)
    const TArray<class ABall*>& GetMovingBalls() const { return MovingBalls; }
    UFUNCTION(BlueprintPure)
    const TArray<class ABall*>& GetActiveBalls() const { return ActiveBalls; }
    UFUNCTION(BlueprintPure)
    const TArray<class ABall*>& GetPocketedBalls() const { return PocketedBalls; }
    UFUNCTION(BlueprintPure)
    const TArray<class ABall*>& GetDroppedBalls() const { return DroppedBalls; }
    UFUNCTION(BlueprintPure)
    const TArray<class ABall*>& GetBallsPlayedOut() const { return BallsPlayedOutOfGame; }

protected:
    UPROPERTY(Replicated)
    TArray<class ABall*> MovingBalls;
    UPROPERTY(Replicated)
    TArray<class ABall*> ActiveBalls;
    UPROPERTY(Replicated)
    TArray<class ABall*> PocketedBalls;
    UPROPERTY(Replicated)
    TArray<class ABall*> BallsHittedByTheCue;
    UPROPERTY(Replicated)
    TArray<class ABall*> DroppedBalls;
    UPROPERTY(Replicated)
    TArray<class ABall*> BallsPlayedOutOfGame;
};
