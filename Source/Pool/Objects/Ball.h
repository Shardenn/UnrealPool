// Copyright 2019 Andrei Vikarchuk.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Objects/BallSpecialState.h"
#include "Ball.generated.h"

UCLASS()
class POOL_API ABall : public AActor
{
    GENERATED_BODY()

public:
    ABall();

    void SetSpecialBallState(FSpecialBallState SpecialState);
protected:
    virtual void BeginPlay() override;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FVector SpawnLocation = FVector(0);

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    UStaticMeshComponent* SphereMesh;

    UFUNCTION(BlueprintImplementableEvent)
    void SetupSpecialState(FSpecialBallState SpecialState);
};
