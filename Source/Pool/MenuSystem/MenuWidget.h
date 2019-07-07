// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MenuInterface.h"
#include "MenuWidget.generated.h"

/**
 * 
 */
UCLASS()
class POOL_API UMenuWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
    void SetMenuInterface(IMenuInterface* NewInterface);

    void Setup();
    void Teardown();

protected:
    virtual bool Initialize() override;
    IMenuInterface* MenuInterface;
};
