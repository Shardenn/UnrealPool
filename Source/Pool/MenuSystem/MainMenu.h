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
public:
    UMainMenu(const FObjectInitializer& ObjectInitializer);

    void SetServerList(TArray<FString> ServerNames);

    void SelectIndex(uint32 Index);
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

    // scroll box of servers
    UPROPERTY(meta = (BindWidget))
    class UPanelWidget* ServerList;

    TSubclassOf<class UUserWidget> ServerRowClass;

    //class UServerRow* ServerRow;
    
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

    TOptional<uint32> SelectedServerIndex;
};
