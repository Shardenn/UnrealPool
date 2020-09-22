// Copyright 2019 Andrei Vikarchuk.

#pragma once

#include "CoreMinimal.h"
#include "Player/Billiardist/BilliardistPawn.h"
#include "Player/Interfaces/BilliardistWithPlacableBall.h"
#include "GameplayLogic/Interfaces/BallInHandUpdateListener.h"
#include "BilliardistPawnWithPlacableBall.generated.h"

/**
 *
 */
UCLASS()
class POOL_API ABilliardistPawnWithPlacableBall : public ABilliardistPawn, public IBilliardistWithPlacableBall, public IBallInHandUpdateListener
{
    GENERATED_BODY()
public:
    virtual void TryPlaceBall(const TScriptInterface<IPlayerWithHandableBall>& Player) override;
    virtual void OnBallInHandUpdate(class ABall* const Ball) override;

protected:
    virtual void Tick(float DeltaTime) override;

    bool IsBallPlacementValid() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<class ABall> GhostBallClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    class ABall* GhostHandedBall = nullptr;

    virtual void ActionReleaseHandle() override;

    void BeginPlay() override;
    void SubscribeToBallInHandUpdate();
private:
    TScriptInterface<IPlayerWithHandableBall> HandablePlayer;
    FVector PreviousGhostBallLocation{ 0 };
};
