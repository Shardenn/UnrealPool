// Copyright 2019 Andrei Vikarchuk.

#pragma once

#include "CoreMinimal.h"
#include "Components/CapsuleComponent.h"
#include "PocketArea.generated.h"

class UStaticMeshComponent;

/**
 *
 */
UCLASS()
class POOL_API UPocketArea : public UCapsuleComponent
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintImplementableEvent)
    void OnHoverOver();
    UFUNCTION(BlueprintImplementableEvent)
    void OnStopHoverOver();
    UFUNCTION(BlueprintImplementableEvent)
    void OnSelect();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UStaticMeshComponent* MeshToHighlight { nullptr };
};
