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

    UFUNCTION()
    void OnTurnEnd();

    void OnFrameRestarted();
    // they are attached to awake and sleep events in ABall class.
    // NOTE And they are attached only on the server.
    // So the function is only called on server.
    // Only works if balls are correctly processed on server
    // (like LaunchBall is called as server function)
    void AddMovingBall(ABall* Ball);
    void RemoveMovingBall(ABall* Ball);

    void AddBallPocketedDuringTurn(ABall* Ball);

    void AddBallDroppedDuringTurn(ABall* Ball);

    void AddBallHittedByTheCue(ABall* Ball);

    UFUNCTION(BlueprintPure)
    const TArray<ABall*>& GetMovingBalls() const { return MovingBalls; }
    UFUNCTION(BlueprintPure)
    const TArray<ABall*>& GetActiveBalls() const { return ActiveBalls; }
    UFUNCTION(BlueprintPure)
    const TArray<ABall*>& GetPocketedBalls() const { return PocketedBalls; }
    UFUNCTION(BlueprintPure)
    const TArray<ABall*>& GetBallsPocketedDuringTurn() const { return BallsPocketedDuringTurn; }
    UFUNCTION(BlueprintPure)
    const TArray<ABall*>& GetDroppedBalls() const { return DroppedBalls; }
    UFUNCTION(BlueprintPure)
    const TArray<ABall*>& GetBallsDroppedDuringTurn() const { return BallsDroppedDuringTurn; }
    UFUNCTION(BlueprintPure)
    const TArray<ABall*>& GetBallsPlayedOut() const { return BallsPlayedOutOfGame; }
    UFUNCTION(BlueprintPure)
    const TArray<ABall*>& GetBallsHittedByTheCue() const { return BallsHittedByTheCue; }

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

    UPROPERTY(Replicated)
    TArray<ABall*> BallsPocketedDuringTurn;
    UPROPERTY(Replicated)
    TArray<ABall*> BallsDroppedDuringTurn;

    //virtual void PostInitProperties() override;

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_OnBallPocketed(const ABall* Ball);

    UFUNCTION(Client, Reliable)
    void Client_OnFrameRestarted();
private:
    // Clears turn-relevant arrays 
    void ResetTurnArrays();
};
