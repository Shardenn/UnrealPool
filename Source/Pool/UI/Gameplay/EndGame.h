// Copyright 2019 Andrei Vikarchuk.

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetBase.h"
#include "EndGame.generated.h"

/**
 * 
 */
UCLASS()
class POOL_API UEndGame : public UWidgetBase
{
    GENERATED_BODY()

protected:
    virtual bool Initialize() override;

    UPROPERTY(meta = (BindWidget))
    class UCheckBox* CheckRematchVote;
};
