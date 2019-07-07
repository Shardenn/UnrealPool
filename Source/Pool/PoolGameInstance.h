// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MenuSystem/MenuInterface.h"
#include "PoolGameInstance.generated.h"

/**
 *
 */
UCLASS()
class POOL_API UPoolGameInstance : public UGameInstance, public IMenuInterface
{
    GENERATED_BODY()

public:
    UPoolGameInstance(const FObjectInitializer& ObjectInitializer);

    virtual void Init();

    UFUNCTION(BlueprintCallable)
    void LoadMenu();

    UFUNCTION(BlueprintCallable)
    void LoadMenuInGame();

    UFUNCTION(Exec)
    void Host() override;

    UFUNCTION(Exec)
    void Join(const FString& Address) override;

    void LoadMainMenuLevel() override;

private:
    TSubclassOf<class UUserWidget> MainMenuClass;
    TSubclassOf<class UUserWidget> InGameMenuClass;

    class UMainMenu* Menu;
};
