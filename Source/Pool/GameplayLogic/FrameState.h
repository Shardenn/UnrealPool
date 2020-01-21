// Copyright 2019 Andrei Vikarchuk.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "FrameState.generated.h"

/**
 *
 */
UCLASS()
class POOL_API UFrameState : public UObject
{
    GENERATED_BODY()

public:
    UFrameState();
    virtual ~UFrameState();

    UFUNCTION(BlueprintPure)
    const TArray<class ABall*>& GetActiveBalls() const { return m_ActiveBalls; }
    UFUNCTION(BlueprintPure)
    const TArray<class ABall*>& GetDroppedBalls() const { return m_DroppedBalls; }
    UFUNCTION(BlueprintPure)
    const TArray<class ABall*>& GetPocketedBalls() const { return m_PocketedBalls; }
    UFUNCTION(BlueprintPure)
    const TArray<class ABall*>& GetBallsPlayedOut() const { return m_BallsPlayedOutOfGame; }

    UFUNCTION()
    virtual void Restart();

    // Fascade for clearing vars
    virtual void ClearFrameStateVariables();

    //UFUNCTION(Server, reliable, WithValidation)
    virtual void Server_SwitchTurn();
    
    bool RequestIsPlayerTurn(const class APlayerState* PlayerState);
protected:
    virtual bool IsSupportedForNetworking() const override { return true; }

    virtual void Initialise(class AGameState* InGameState);
    virtual void Server_HandleTurnEnd() = 0;
    // The (sub)class of our turnstate handler
    //TSubclassOf<class UTurnState> TurnStateClass;
    class UTurnStateEightBall* TurnState{ nullptr };

    class AGameState* GameState = nullptr;
    TArray<class APlayerState*> PlayerArray;

    void OnRep_UpdatePlayerStateTurn();

    //UPROPERTY(Replicated/*Using=OnRep_UpdatePlayerStateTurn*/)
    uint32 PlayerIndexTurn = 0;
    //class APoolPlayerState* m_PlayerWithCueBall = nullptr;

    bool m_bTableOpened = true;
    bool m_bBallsRackBroken = false;
    bool m_bShouldSwitchTurn = true;

    // Balls still on the table
    TArray<class ABall*> m_ActiveBalls;
    // Balls that were played out by dropping
    TArray<class ABall*> m_DroppedBalls;
    // Balls that were played out by pocketing
    TArray<class ABall*> m_PocketedBalls;
    // Balls that just were played out = dropped + pocketed
    TArray<class ABall*> m_BallsPlayedOutOfGame;

    void RegisterBall(ABall* InBall) { m_BallsPlayedOutOfGame.AddUnique(InBall); }
private:
    // Removes the ball from the active ones
    //UFUNCTION(Server, Reliable, WithValidation)
    void Server_RegisterBall(class ABall* Ball);
};
