// Copyright 2019 Andrei Vikarchuk.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Cue.generated.h"

UCLASS()
class POOL_API ACue : public AActor
{
    GENERATED_BODY()

public:
    ACue();

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class UStaticMeshComponent* Mesh{ nullptr };
};
