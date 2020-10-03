// Copyright 2019 Andrei Vikarchuk.

#pragma once

#include "CoreMinimal.h"
#include "Player/Billiardist/BilliardistPawnWithPlacableBall.h"
#include "Player/Interfaces/BilliardistWithNamedShot.h"
#include "EightBallBilliardist.generated.h"

/**
 *
 */
UCLASS()
class POOL_API AEightBallBilliardist : public ABilliardistPawnWithPlacableBall, public IBilliardistWithNamedShot
{
    GENERATED_BODY()

protected:
    virtual void NameShot(UPocketArea* SelectedPocket, ABall* SelectedBall) override;
    virtual void PredictShot(UPocketArea* SelectedPocket, ABall* SelectedBall) override;

    virtual void ActionPressHandle() override;
    virtual void ActionReleaseHandle() override;

    virtual void Tick(float DeltaTime) override;

    virtual void OnRep_PlayerState() override;
    virtual void PossessedBy(AController* NewController) override;

    UPocketArea* SelectedPocket{ nullptr };

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PredictedShotCastLength{ 300.f };

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PredictedPocketConeAngle{ 30.f };
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PredictedPocketConeStep{ 3.f };
private:
    void DrawPredictedLine();
    // LookDirection with zero Z
    class ABall* GetBallInLineOfSight(const FVector& StartLocation, const FVector& LookDirection, 
        const float BallRadius, FVector& ImpactPoint);
    class UPocketArea* GetPocketInLineOfShot(const FVector& StartLocation, const FVector& RollDirection);
    // We will use it every frame in certain situations.
    // Better keep it as a field for saving performance
    class APlayerCameraManager* CamManager;

    class UPocketArea* LastPredictedPocket{ nullptr };
    class ABallAmerican* LastPredictedBall{ nullptr };
};
