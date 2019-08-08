// Copyright 2019 Andrei Vikarchuk.

#pragma once

#include "CoreMinimal.h"
#include "MenuWidget.h"
#include "InGameMenu.generated.h"

/**
 * 
 */
UCLASS()
class POOL_API UInGameMenu : public UMenuWidget
{
    GENERATED_BODY()

protected:
    virtual bool Initialize() override;

private:
    UPROPERTY(meta = (BindWidget))
    class UButton* ButtonCancel;

    UPROPERTY(meta = (BindWidget))
    class UButton* ButtonQuit;

    UFUNCTION()
    void HideInGameMenu();

    UFUNCTION()
    void QuitPressed();
};
