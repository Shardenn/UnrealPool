#include "PoolGameInstance.h"
#include "Pool.h"
#include "MenuSystem/MainMenu.h"
#include "MenuSystem/MenuWidget.h"
#include "MenuSystem/InGameMenu.h"

#include "OnlineSubsystem.h"
#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"

#include "Blueprint/UserWidget.h"

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
        auto SessionInterface = Subsystem->GetSessionInterface();
        if (SessionInterface.IsValid())
        {
            UE_LOG(LogPool, Warning, TEXT("Found session interface"));
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

void UPoolGameInstance::Host()
{
    UEngine* Engine = GetEngine();
    if (!ensure(Engine != nullptr)) return;

    Engine->AddOnScreenDebugMessage(0, 2, FColor::Green, TEXT("Hosting"));

    UWorld* World = GetWorld();
    if (!ensure(World != nullptr)) return;

    if (Menu)
        Menu->Teardown();

    World->ServerTravel("/Game/Maps/Pool_cozy_room?listen");
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
