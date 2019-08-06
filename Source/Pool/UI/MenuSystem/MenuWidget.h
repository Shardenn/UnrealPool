// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetBase.h"
#include "MenuInterface.h"
#include "MenuWidget.generated.h"

/**
 * 
 */
UCLASS()
class POOL_API UMenuWidget : public UWidgetBase
{
	GENERATED_BODY()
	
public:
    void SetMenuInterface(IMenuInterface* NewInterface);

protected:
    IMenuInterface* MenuInterface;
};
