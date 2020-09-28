// Copyright 2019 Andrei Vikarchuk.

#pragma once

#include "CoreMinimal.h"
#include "Player/Billiardist/BilliardistPawn.h"
#include "Player/Interfaces/BilliardistWithPlacableBall.h"
//#include "GameplayLogic/Interfaces/BallInHandUpdateListener.h"
#include "BilliardistPawnWithPlacableBall.generated.h"

class ABall;

/**
 *
 */
UCLASS()
class POOL_API ABilliardistPawnWithPlacableBall : public ABilliardistPawn, public IBilliardistWithPlacableBall//, public IBallInHandUpdateListener
{
    GENERATED_BODY()
public:
    virtual void TryPlaceBall(const TScriptInterface<IPlayerWithHandableBall>& Player) override;

protected:
    virtual void Tick(float DeltaTime) override;

    bool IsBallPlacementValid() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<ABall> GhostBallClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
    ABall* GhostHandedBall = nullptr;

    virtual void ActionReleaseHandle() override;

    void BeginPlay() override;
    void SubscribeToBallInHandUpdate();

    virtual void SetBallInHand(ABall* Ball, bool bInitialPlacement) override;

    virtual void OnRep_PlayerState() override;
    virtual void PossessedBy(AController* NewController) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
    bool bInitialPlacement{ false };
private:
    class APSWithHandableBall* HandablePlayer;
    FVector LastSuccessfullGhostBallLocation{ 1000, 100, 100 };

    class ABilliardistController* MyController{ nullptr };

    UFUNCTION()
    void OnBallInHandUpdate(ABall* Ball, bool bInitialPlacementIn = false);

    UFUNCTION(Client, Reliable)
    void Client_UpdateGhostBallLocation();
    UFUNCTION(Server, Unreliable, WithValidation)
    void Server_SetGhostBallLocation(const FVector& NewLocaiton);
};
