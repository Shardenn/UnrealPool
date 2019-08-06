// Copyright 2019 Andrei Vikarchuk.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WidgetBase.generated.h"

/**
 *
 */
UCLASS()
class POOL_API UWidgetBase : public UUserWidget
{
    GENERATED_BODY()

public:
    void Setup();
    void Teardown();

protected:
    virtual bool Initialize() override;
};
