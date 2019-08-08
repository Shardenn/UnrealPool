#include "PoolGameInstance.h"
#include "Pool.h"
#include "UI/MenuSystem/MainMenu.h"
#include "UI/MenuSystem/MenuWidget.h"
#include "UI/MenuSystem/InGameMenu.h"

#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"
#include "OnlineSessionSettings.h"

#include "Blueprint/UserWidget.h"

const static FName SESSION_NAME = TEXT("My_session_game");
const static FName SERVER_NAME_SETTING_KEY = TEXT("ServerName");

UPoolGameInstance::UPoolGameInstance(const FObjectInitializer& ObjectInitializer)
{
    ConstructorHelpers::FClassFinder<UMainMenu> MainMenuBPClass(TEXT("/Game/UI/MenuSystem/WBP_MainMenu"));
    if (!ensure(MainMenuBPClass.Class != nullptr)) return;

    ConstructorHelpers::FClassFinder<UInGameMenu> InGameBPClass(TEXT("/Game/UI/MenuSystem/WBP_InGameMenu"));
    if (!ensure(InGameBPClass.Class != nullptr)) return;

    // Store MainMenu class for displaying later
    MainMenuClass = MainMenuBPClass.Class;
    InGameMenuClass = InGameBPClass.Class;
}

void UPoolGameInstance::Init()
{
    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
    if (Subsystem != nullptr)
    {
        UE_LOG(LogPool, Warning, TEXT("Found online subsystem named %s"), *Subsystem->GetSubsystemName().ToString());
        SessionInterface = Subsystem->GetSessionInterface();
        if (SessionInterface.IsValid())
        {
            SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UPoolGameInstance::OnSessionCreated);
            SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UPoolGameInstance::OnSessionDestroy);
            SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UPoolGameInstance::OnSessionsSearchComplete);
            SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UPoolGameInstance::OnSessionJoinComplete);
        }
    }
    else
    {
        UE_LOG(LogPool, Error, TEXT("No online subsystem found"));
    }
}

void UPoolGameInstance::LoadMenu()
{
    if (!ensure(MainMenuClass != nullptr)) return;

    Menu = CreateWidget<UMainMenu>(this, MainMenuClass);
    if (!ensure(Menu != nullptr)) return;

    Menu->Setup();
    Menu->SetMenuInterface(this);
}

void UPoolGameInstance::LoadMenuInGame()
{
    if (!ensure(InGameMenuClass != nullptr)) return;

    UMenuWidget* InGameMenu = CreateWidget<UMenuWidget>(this, InGameMenuClass);
    if (!ensure(InGameMenu != nullptr)) return;

    InGameMenu->Setup();
    InGameMenu->SetMenuInterface(this);
}

void UPoolGameInstance::OnSessionCreated(FName Name, bool bSuccess)
{
    if (!bSuccess)
    {
        UE_LOG(LogPool, Error, TEXT("Session could not be created"));
        return;
    }

    UEngine* Engine = GetEngine();
    if (!ensure(Engine != nullptr)) return;

    Engine->AddOnScreenDebugMessage(0, 2, FColor::Green, TEXT("Hosting"));

    UWorld* World = GetWorld();
    if (!ensure(World != nullptr)) return;

    if (Menu)
        Menu->Teardown();

    World->ServerTravel("/Game/Maps/Pool_cozy_room?listen");
}

void UPoolGameInstance::OnSessionDestroy(FName Name, bool bSuccess)
{
    if (bSuccess)
    {
        CreateSession();
    }
}

void UPoolGameInstance::OnSessionsSearchComplete(bool bFound)
{
    if (SessionSearch.IsValid() && bFound)
    {
        if (SessionSearch->SearchResults.Num() == 0)
        {
            UE_LOG(LogPool, Warning, TEXT("No sessions found"));
            return;
        }

        TArray<FServerData> ServerNames;
        for (const auto& SessionResult : SessionSearch->SearchResults)
        {
            UE_LOG(LogPool, Warning, TEXT("Found session %s"), *SessionResult.GetSessionIdStr());
            FServerData Data;
            Data.MaxPlayers = SessionResult.Session.SessionSettings.NumPublicConnections;
            Data.CurrentPlayers = Data.MaxPlayers - SessionResult.Session.NumOpenPublicConnections;
            Data.HostUsername = SessionResult.Session.OwningUserName;
            FString GotServerName;
            if (SessionResult.Session.SessionSettings.Get(SERVER_NAME_SETTING_KEY, GotServerName))
            {
                Data.Name = GotServerName;
            }
            else
            {
                Data.Name = "Could not get name";
            }

            ServerNames.Add(Data);
        }

        Menu->SetServerList(ServerNames);
    }
}

void UPoolGameInstance::OnSessionJoinComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
    FString TravelURL;
    UE_LOG(LogPool, Warning, TEXT("Found session name is %s"), *SessionName.ToString());

    if (!SessionInterface->GetResolvedConnectString(SessionName, TravelURL))
    {
        UE_LOG(LogPool, Warning, TEXT("Could not get resolved connect string"));
        return;
    }

    APlayerController* PlayerController = GetFirstLocalPlayerController();
    if (!ensure(PlayerController != nullptr)) return;

    PlayerController->ClientTravel(TravelURL, ETravelType::TRAVEL_Absolute);
}

void UPoolGameInstance::CreateSession()
{
    if (SessionInterface.IsValid())
    {
        FOnlineSessionSettings SessionSettings;
        if (IOnlineSubsystem::Get()->GetSubsystemName() == "NULL")
        {
            SessionSettings.bIsLANMatch = true;
        }
        else
        {
            SessionSettings.bIsLANMatch = false;
        }
        SessionSettings.NumPublicConnections = 2;
        SessionSettings.bShouldAdvertise = true; // visible via search
        SessionSettings.bUsesPresence = true;
        SessionSettings.Set(SERVER_NAME_SETTING_KEY, DesiredServerName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

        SessionInterface->CreateSession(0, SESSION_NAME, SessionSettings);
    }
}

void UPoolGameInstance::Host(FString ServerName)
{
    DesiredServerName = ServerName;
    if (SessionInterface.IsValid())
    {
        auto ExistingSession = SessionInterface->GetNamedSession(SESSION_NAME);
        if (ExistingSession != nullptr)
        {
            // it will automatically call CreateSession, see OnSessionDestroy above
            SessionInterface->DestroySession(SESSION_NAME);
        }
        else
        {
            CreateSession();
        }
    }
}

void UPoolGameInstance::Join(uint32 Index)
{
    if (!SessionInterface.IsValid()) return;

    if (Menu)
    {
        Menu->Teardown();
    }

    if (!SessionSearch.IsValid()) return;
    SessionInterface->JoinSession(0, SESSION_NAME, SessionSearch->SearchResults[Index]);
}

void UPoolGameInstance::RequestFindSessions()
{
    // search for sessions
    SessionSearch = MakeShareable(new FOnlineSessionSearch());
    if (SessionSearch.IsValid())
    {
        if (IOnlineSubsystem::Get()->GetSubsystemName() == "NULL")
            SessionSearch->bIsLanQuery = true;
        else
            SessionSearch->bIsLanQuery = false;

        SessionSearch->MaxSearchResults = 10000;
        // for steam presence enabled
        SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
        UE_LOG(LogPool, Warning, TEXT("Starting searching for sessions..."));
        SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
    }
}

void UPoolGameInstance::LoadMainMenuLevel()
{
    APlayerController* PlayerController = GetFirstLocalPlayerController();
    if (!ensure(PlayerController != nullptr)) return;

    PlayerController->ClientTravel("/Game/Maps/MainMenu", ETravelType::TRAVEL_Absolute);
}
