// Copyright 2019 Andrei Vikarchuk.

#pragma once

#include "CoreMinimal.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "PocketArea.generated.h"

/**
 *
 */
UCLASS(Blueprintable, ClassGroup = (TableComponents))//, meta = (BlueprintSpawnableComponent))
class POOL_API UPocketArea : public UCapsuleComponent
{
    GENERATED_BODY()

public:
    virtual void BeginPlay() override;

    UFUNCTION(BlueprintNativeEvent)
    void OnHoverOver();
    UFUNCTION(BlueprintNativeEvent)
    void OnStopHoverOver();
    UFUNCTION(BlueprintNativeEvent)
    void OnSelected();
    UFUNCTION(BlueprintNativeEvent)
    void OnDeselected();

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class UStaticMeshComponent* MeshToHighlight { nullptr };

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName PocketTag { "Pocket" };

private:
    UFUNCTION()
    void OnTurnEndFired();
};
