// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MenuWidget.h"

#include "MainMenu.generated.h"

/**
 *
 */
UCLASS()
class POOL_API UMainMenu : public UMenuWidget
{
    GENERATED_BODY()
protected:
    virtual bool Initialize() override;
    virtual void OnLevelRemovedFromWorld(class ULevel* InLevel, 
                                         class UWorld* InWorld) override;
private:
    UPROPERTY(meta = (BindWidget))
    class UButton* ButtonHost;
    UPROPERTY(meta = (BindWidget))
    class UButton* ButtonJoin;
    UPROPERTY(meta = (BindWidget))
    class UButton* ButtonQuitGame;

    UPROPERTY(meta = (BindWidget))
    class UButton* ButtonJoinCancel;
    UPROPERTY(meta = (BindWidget))
    class UButton* ButtonJoinConfirm;

    UPROPERTY(meta = (BindWidget))
    class UEditableText* AddressField;
    
    UPROPERTY(meta = (BindWidget))
    class UWidgetSwitcher* MenuSwitcher;

    UPROPERTY(meta = (BindWidget))
    class UWidget* MainMenu;

    UPROPERTY(meta = (BindWidget))
    class UWidget* JoinMenu;
    
    UFUNCTION()
    void HostServer();
    UFUNCTION()
    void JoinServer();

    UFUNCTION()
    void OpenJoinMenu();
    UFUNCTION()
    void OpenMainMenu();

    UFUNCTION()
    void QuitPressed();
};
