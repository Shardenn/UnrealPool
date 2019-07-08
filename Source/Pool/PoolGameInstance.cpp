#include "PoolGameInstance.h"
#include "Pool.h"
#include "MenuSystem/MainMenu.h"
#include "MenuSystem/MenuWidget.h"
#include "MenuSystem/InGameMenu.h"

#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"
#include "OnlineSessionSettings.h"
#include "OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"

#include "Blueprint/UserWidget.h"

const static FName SESSION_NAME = TEXT("My session game");

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
    UE_LOG(LogPool, Warning, TEXT("PoolGameInstance::Init()"));
    UE_LOG(LogPool, Warning, TEXT("Found MenuClass %s"), *MainMenuClass.Get()->GetName());

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

            // search for sessions
            SessionSearch = MakeShareable(new FOnlineSessionSearch());
            if (SessionSearch.IsValid())
            {
                UE_LOG(LogPool, Warning, TEXT("Starting searching for sessions..."));
                SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
            }
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
    if (bFound)
    {
        UE_LOG(LogPool, Warning, TEXT("Found sessions"));
    }
    else
    {
        UE_LOG(LogPool, Warning, TEXT("Did not find sessions"));
    }
}

void UPoolGameInstance::CreateSession()
{
    if (SessionInterface.IsValid())
    {
        FOnlineSessionSettings SessionSettings;
        SessionInterface->CreateSession(0, SESSION_NAME, SessionSettings);
    }
}

void UPoolGameInstance::Host()
{
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

void UPoolGameInstance::Join(const FString& Address)
{
    UEngine* Engine = GetEngine();
    if (!ensure(Engine != nullptr)) return;

    Engine->AddOnScreenDebugMessage(0, 5, FColor::Green, FString::Printf(TEXT("Joining %s"), *Address));

    APlayerController* PlayerController = GetFirstLocalPlayerController();
    if (!ensure(PlayerController != nullptr)) return;

    if (Menu)
        Menu->Teardown();

    PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
}

void UPoolGameInstance::LoadMainMenuLevel()
{
    APlayerController* PlayerController = GetFirstLocalPlayerController();
    if (!ensure(PlayerController != nullptr)) return;

    PlayerController->ClientTravel("/Game/Maps/MainMenu", ETravelType::TRAVEL_Absolute);
}
