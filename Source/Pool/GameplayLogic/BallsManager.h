// Copyright 2019 Andrei Vikarchuk.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Pool.h"
#include "BallsManager.generated.h"

class ABall;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBallPocketed, const ABall*, Ball);
/**
 *
 */

UCLASS(Blueprintable)
class POOL_API UBallsManager : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnBallPocketed OnBallPocketed;

    virtual bool IsSupportedForNetworking() const override { return true; }
    virtual bool CallRemoteFunction(UFunction* Function, void* Parms, struct FOutParmRec* OutParms, FFrame* Stack) override;
    virtual int32 GetFunctionCallspace(UFunction* Function, FFrame* Stack) override;

    // Clears all arrays
    void Reset();

    // they are attached to awake and sleep events in ABall class.
    // NOTE And they are attached only on the server.
    // So the function is only called on server.
    // Only works if balls are correctly processed on server
    // (like LaunchBall is called as server function)
    void AddMovingBall(ABall* Ball);
    void RemoveMovingBall(ABall* Ball);

    void AddPocketedBall(ABall* Ball);

    void AddDroppedBall(ABall* Ball);
    /*
    void OnCueBallHit(UPrimitiveComponent* HitComponent,
            AActor* OtherActor,
            UPrimitiveComponent* OtherComp,
            FVector NormalImpulse,
            const FHitResult& Hit);
            */

    UFUNCTION(BlueprintPure)
    const TArray<ABall*>& GetMovingBalls() const { return MovingBalls; }
    UFUNCTION(BlueprintPure)
    const TArray<ABall*>& GetActiveBalls() const { return ActiveBalls; }
    UFUNCTION(BlueprintPure)
    const TArray<ABall*>& GetPocketedBalls() const { return PocketedBalls; }
    UFUNCTION(BlueprintPure)
    const TArray<ABall*>& GetDroppedBalls() const { return DroppedBalls; }
    UFUNCTION(BlueprintPure)
    const TArray<ABall*>& GetBallsPlayedOut() const { return BallsPlayedOutOfGame; }

protected:
    UPROPERTY(Replicated)
    TArray<ABall*> MovingBalls;
    UPROPERTY(Replicated)
    TArray<ABall*> ActiveBalls;
    UPROPERTY(Replicated)
    TArray<ABall*> PocketedBalls;
    UPROPERTY(Replicated)
    TArray<ABall*> BallsHittedByTheCue;
    UPROPERTY(Replicated)
    TArray<ABall*> DroppedBalls;
    UPROPERTY(Replicated)
    TArray<ABall*> BallsPlayedOutOfGame;

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_OnBallPocketed(const ABall* Ball);
};
