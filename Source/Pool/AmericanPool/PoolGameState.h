// Copyright 2019 Andrei Vikarchuk.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "PoolGameState.generated.h"

/**
 *
 */
UCLASS()
class POOL_API APoolGameState : public AGameState
{
    GENERATED_BODY()

public:
    APoolGameState();

    UFUNCTION(Server, Reliable, WithValidation)
    void SetPlayersReadyNum(uint32 PlayersReady);

    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
    int32 PlayersReadyNum = 0;

    UFUNCTION()
    void OnFrameRestarted();

    UFUNCTION(Server, Reliable, WithValidation)
    void HandleTurnEnd();

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_GiveBallInHand(APoolPlayerState* PlayerState = nullptr);

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_TakeBallFromHand();

    bool RequestIsPlayerTurn(APlayerState* PlayerState);

    class ABall* const GetCueBall();

    UFUNCTION(BlueprintPure)
    APoolPlayerState* GetOtherPlayerState(const APoolPlayerState* Mine);
protected:
    virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

    virtual void BeginPlay() override;

    // returns true if the conditions for the win
    // are satisfied. (Like every ball is pocketed
    // BEFORE pocketed 8 ball).
    virtual bool DecideWinCondition();

    //UPROPERTY(Replicated)
    //class ABallAmerican* CueBall = nullptr;

    TSubclassOf<class UFrameState> FrameStateClass;
    class UFrameStateEightBall* FrameState = nullptr;
private:
    void OnRep_UpdatePlayerStateTurn();

    bool FindAndInitializeCueBall();
};
