// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "NetworkReplayStreaming.h"
#include "Engine/GameInstance.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionInterface.h"

#include "UI/MenuSystem/MenuInterface.h"

#include "PoolGameInstance.generated.h"

USTRUCT(BlueprintType)
struct FReplayInfo
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString ReplayName;
    UPROPERTY(BlueprintReadOnly)
    FString FriendlyName;
    UPROPERTY(BlueprintReadOnly)
    FDateTime Timestamp;
    UPROPERTY(BlueprintReadOnly)
    int32 LengthInMS;
    UPROPERTY(BlueprintReadOnly)
    bool bIsValid;

    FReplayInfo()
    {
        ReplayName = "Replay";
        FriendlyName = "Replay";
        Timestamp = FDateTime::MinValue();
        LengthInMS = 0;
        bIsValid = false;
    }

    FReplayInfo(FString NewName, FString NewFriendlyName, FDateTime NewTimestamp, int32 NewLengthInMS)
    {
        ReplayName = NewName;
        FriendlyName = NewFriendlyName;
        Timestamp = NewTimestamp;
        LengthInMS = NewLengthInMS;
    }
};
/**
 *
 */
UCLASS()
class POOL_API UPoolGameInstance : public UGameInstance, public IMenuInterface
{
    GENERATED_BODY()

public:
    UPoolGameInstance(const FObjectInitializer& ObjectInitializer);

    virtual void Init() override;

    UFUNCTION(BlueprintCallable)
    void LoadMenu();

    UFUNCTION(BlueprintCallable)
    void LoadMenuInGame();

    UFUNCTION(Exec)
    void Host(FString ServerName) override;

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

    FString DesiredServerName;
    void CreateSession();

    TSharedPtr<class FOnlineSessionSearch> SessionSearch;

    class UMainMenu* Menu;

///////////////////////////////
// Replay
    FString LastRecordingName;
public:
    /* Start recording the replay.
     * return: nothing
     * param ReplayName: name on disk
     * param FriendlyName: name for UI
     */
    UFUNCTION(BlueprintCallable, Category = "Replays")
    void StartRecording(FString ReplayName, FString FriendlyName);
    /* Stop the running replay and save it */
    UFUNCTION(BlueprintCallable, Category = "Replays") 
    void StopRecording();
    /* Start playback for the latest replay */
    UFUNCTION(BlueprintCallable, Category = "Replays")
    void PlayLastRecording(FString ReplayName);
    /* Start looking for the replays on the hard drive */
    UFUNCTION(BlueprintCallable, Category = "Replays")
    void FindReplays();
    /* Apply a custom name for the replay (for UI) */
    UFUNCTION(BlueprintCallable, Category = "Replays")
    void RenameReplay(const FString& ReplayName, const FString& NewFriendlyReplayName);
    UFUNCTION(BlueprintCallable, Category = "Replays")
    void DeleteReplay(const FString& ReplayName);

protected:
    UFUNCTION(BlueprintImplementableEvent, Category = "Replays")
    void BP_OnFindReplaysComplete(const TArray<FReplayInfo>& AllReplays);
private:
    TSharedPtr<INetworkReplayStreamer> EnumerateStreamsPtr;
    // For FindReplays()
    FEnumerateStreamsCallback OnEnumerateStreamsCallback;
    void OnEnumerateStreamsComplete(const FEnumerateStreamsResult& StreamInfos);

    // for DeleteReplay(..)
    FDeleteFinishedStreamCallback OnDeleteFinishedStreamCompleteCallback;

    void OnDeleteFinishedStreamComplete(const FDeleteFinishedStreamResult& Result);
};
