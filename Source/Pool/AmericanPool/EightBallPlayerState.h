// Copyright 2019 Andrei Vikarchuk.

#pragma once

#include "CoreMinimal.h"
#include "GameplayLogic/PSWithHandableBall.h"

#include "GameplayLogic/Interfaces/PlayerWithMainCueBall.h"
#include "GameplayLogic/Interfaces/PlayerWithNamedShot.h"

#include "EightBallPlayerState.generated.h"

/**
 *
 */
UCLASS()
class POOL_API AEightBallPlayerState : public APSWithHandableBall, public IPlayerWithMainCueBall, public IPlayerWithNamedShot
{
    GENERATED_BODY()

public:
    virtual ABall* GetCueBall() override;
    virtual void NameShot(UPocketArea* SelectedPocket, ABall* SelectedBall) override;
    virtual void PredictShot(UPocketArea* SelectedPocket, ABall* SelectedBall) override;
    
    void AssignBallType(const FBallType& Type) noexcept { AssignedBallType = Type; }

    UFUNCTION(BlueprintPure)
    FBallType GetAssignedBallType() const noexcept { return AssignedBallType; }
protected:
    UPROPERTY(Replicated)
    FBallType AssignedBallType { FBallType::NotInitialized };

    UPROPERTY(Replicated)
    ABall* CueBall { nullptr };

    virtual void OnFrameRestarted_Internal() override;

    virtual void NameShot_Internal(UPocketArea* SelectedPocket, ABall* SelectedBall) override;
    virtual void PredictShot_Internal(UPocketArea* SelectedPocket, ABall* SelectedBall) override;

private:
    UFUNCTION(Server, Reliable, WithValidation)
    void Server_NameShot(UPocketArea* SelectedPocket, ABall* SelectedBall);
    UFUNCTION(Server, Reliable, WithValidation)
    void Server_PredictShot(UPocketArea* SelectedPocket, ABall* SelectedBall);
};
