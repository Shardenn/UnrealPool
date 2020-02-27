// Copyright 2019 Andrei Vikarchuk.

#pragma once

#include "CoreMinimal.h"

#include "GameplayLogic/TurnBasedGameState.h"
#include "GameplayLogic/Interfaces/RulesHandler.h"

#include "PoolGameState.generated.h"

UENUM(BlueprintType)
enum class FBallPlayOutReason : uint8
{
    Pocketed      UMETA(DisplayName = "Pocketed"),
    Dropped       UMETA(DisplayName = "Dropped")
};

UENUM(BlueprintType)
enum class FPlayerFoulReason : uint8
{
    CueBallOut          UMETA(DisplayName = "Cue ball out of table"),
    EightBallPocketed   UMETA(DisplayName = "Eight ball pocketed")
};

/**
 *
 */
UCLASS()
class POOL_API APoolGameState : public ATurnBasedGameState, public IRulesHandler
{
    GENERATED_BODY()

public:
    APoolGameState();

    virtual void PostInitializeComponents() override;
    virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

    UFUNCTION(Server, Reliable, WithValidation)
    void SetPlayersReadyNum(uint32 PlayersReady);

    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
    int32 FramesToWin = 0;

    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
    int32 PlayersReadyNum = 0;

    UPROPERTY(BlueprintAssignable)
    FOnPlayerFouled OnPlayerFouled;

    UFUNCTION(BlueprintPure)
    class UBallsManager* const GetBallsManager();
    // not Server. It is only called from LaunchBall, that is 
    // already on Server. Maybe make it server later
    UFUNCTION(Server, Reliable, WithValidation)
    void Server_StartWatchingBallsMovement();

    // they are attached to awake and sleep events in ABall class.
    // NOTE And they are attached only on the server.
    // So the function is only called on server.
    // Only works if balls are correctly processed on server
    // (like LaunchBall is called as server function)
    UFUNCTION()
    void OnBallStartMoving(class UPrimitiveComponent* Comp, FName BoneName);
    UFUNCTION()
    void OnBallStopMoving(class UPrimitiveComponent* Comp, FName BoneName);

    UFUNCTION()
    void OnBallOverlap(UPrimitiveComponent* OverlappedComponent,
            AActor* OtherActor,
            UPrimitiveComponent* OtherComp,
            int32 OtherBodyIndex,
            bool bFromSweep,
            const FHitResult& SweepResult);

    UFUNCTION()
    void OnBallEndOverlap(UPrimitiveComponent* OverlappedComponent,
            AActor* OtherActor,
            UPrimitiveComponent* OtherComp,
            int32 OtherBodyIndex);

    // That function watches for the balls that were hit by the
    // current cue ball. That does NOT mean that the game type has a 
    // mandatory cue ball, like eight ball. That's just the ball
    // that we touch with our cue
    UFUNCTION()
    void OnCueBallHit(UPrimitiveComponent* HitComponent,
            AActor* OtherActor,
            UPrimitiveComponent* OtherComp,
            FVector NormalImpulse,
            const FHitResult& Hit);

    UFUNCTION()
    virtual void OnFrameRestarted();

    virtual void HandleTurnEnd() override;
    virtual void AssignFoul() override;

    virtual bool IsMyTurn(const TScriptInterface<ITurnBasedPlayer>&) override;

    UFUNCTION(BlueprintPure)
    APoolPlayerState* GetOtherPlayerState(const APoolPlayerState* Mine);
protected:
    virtual void BeginPlay() override;

    // Override THIS for defining rules
    virtual void HandleTurnEnd_Internal() override;
    virtual void AssignFoul_Internal() override;
    virtual bool DecideWinCondition();
    virtual void HandlePocketedBall(class ABall* Ball);

    UPROPERTY(Replicated)
    class UBallsManager* BallsManager{ nullptr };

    bool bWatchBallsMovement = false;
    bool bTableOpened = true;
    bool bBallsRackBroken = false;
    bool bPlayerFouled = false;
    bool bShouldSwitchTurn = true;

    void OnRep_UpdatePlayerStateTurn();

    virtual void ClearTurnStateVariables();

private:
    UFUNCTION(Server, Reliable, WithValidation)
    void Server_HandleTurnEnd();
    UFUNCTION(Server, Reliable, WithValidation)
    void Server_AssignFoul();
};
