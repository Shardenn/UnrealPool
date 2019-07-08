// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionInterface.h"

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
    void Join(uint32 Index) override;

    void RequestFindSessions() override;

    void LoadMainMenuLevel() override;

private:
    TSubclassOf<class UUserWidget> MainMenuClass;
    TSubclassOf<class UUserWidget> InGameMenuClass;

    IOnlineSessionPtr SessionInterface;

    void OnSessionCreated(FName Name, bool bSuccess);
    void OnSessionDestroy(FName Name, bool bSuccess);
    void OnSessionsSearchComplete(bool bFound);
    void OnSessionJoinComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

    void CreateSession();

    TSharedPtr<class FOnlineSessionSearch> SessionSearch;

    class UMainMenu* Menu;
};
