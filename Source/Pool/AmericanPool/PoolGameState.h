// Copyright 2019 Andrei Vikarchuk.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerFouled, FPlayerFoulReason, Reason);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTurnEnd);

/**
 *
 */
UCLASS()
class POOL_API APoolGameState : public AGameState
{
    GENERATED_BODY()

public:
    virtual void PostInitializeComponents() override;
    virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

    UFUNCTION(Server, Reliable, WithValidation)
    void SetPlayersReadyNum(uint32 PlayersReady);

    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
    int32 PlayersReadyNum = 0;

    UPROPERTY(BlueprintAssignable)
    FOnTurnEnd OnTurnEnd;
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

    UFUNCTION()
    void OnCueBallHit(UPrimitiveComponent* HitComponent,
            AActor* OtherActor,
            UPrimitiveComponent* OtherComp,
            FVector NormalImpulse,
            const FHitResult& Hit);

    UFUNCTION()
    void OnFrameRestarted();

    UFUNCTION(Server, Reliable, WithValidation)
    void SwitchTurn();

    UFUNCTION(Server, Reliable, WithValidation)
    void HandleTurnEnd();

    UFUNCTION(Server, Reliable, WithValidation)
    void AssignFoul();

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_GiveBallInHand(APoolPlayerState* PlayerState = nullptr);

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_TakeBallFromHand();

    bool RequestIsPlayerTurn(APlayerState* PlayerState);

    // TODO maybe not public?
    
    UPROPERTY(Replicated/*Using=OnRep_UpdatePlayerStateTurn*/)
    uint32 PlayerIndexTurn;

    class ABall* const GetCueBall();

    UFUNCTION(BlueprintPure)
    APoolPlayerState* GetOtherPlayerState(const APoolPlayerState* Mine);
protected:
    virtual void BeginPlay() override;

    // returns true if the conditions for the win
    // are satisfied. (Like every ball is pocketed
    // BEFORE pocketed 8 ball).
    virtual bool DecideWinCondition();

    UPROPERTY(Replicated)
    class ABallAmerican* CueBall = nullptr;

    UPROPERTY(Replicated)
    class UBallsManager* BallsManager{ nullptr };
private:
    bool bWatchBallsMovement = false;
    bool bTableOpened = true;
    bool bBallsRackBroken = false;
    bool bPlayerFouled = false;
    bool bShouldSwitchTurn = true;

    APoolPlayerState* PlayerWithCueBall = nullptr;

    void OnRep_UpdatePlayerStateTurn();

    void ClearTurnStateVariables();

    void HandleBlackBallOutOfPlay();

    bool FindAndInitializeCueBall();
};
