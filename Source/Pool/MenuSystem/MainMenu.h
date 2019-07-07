// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MenuInterface.h"
#include "MainMenu.generated.h"

/**
 *
 */
UCLASS()
class POOL_API UMainMenu : public UUserWidget
{
    GENERATED_BODY()

public:
    void SetMenuInterface(IMenuInterface* NewInterface);

protected:
    virtual bool Initialize() override;

private:
    UPROPERTY(meta = (BindWidget))
    class UButton* ButtonHost;

    UPROPERTY(meta = (BindWidget))
    class UButton* ButtonJoin;

    UFUNCTION()
    void HostServer();

    IMenuInterface* MenuInterface;
};
