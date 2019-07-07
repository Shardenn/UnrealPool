#include "PoolGameInstance.h"
#include "Pool.h"
#include "MenuSystem/MainMenu.h"

#include "OnlineSubsystem.h"
#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"

#include "Blueprint/UserWidget.h"

UPoolGameInstance::UPoolGameInstance(const FObjectInitializer& ObjectInitializer)
{
    ConstructorHelpers::FClassFinder<UMainMenu> MenuBPClass(TEXT("/Game/UI/MenuSystem/WBP_MainMenu"));
    if (!ensure(MenuBPClass.Class != nullptr)) return;

    // Store MainMenu class for displaying later
    MenuClass = MenuBPClass.Class;
}

void UPoolGameInstance::Init()
{
    UE_LOG(LogPool, Warning, TEXT("PoolGameInstance::Init()"));
    UE_LOG(LogPool, Warning, TEXT("Found MenuClass %s"), *MenuClass.Get()->GetName());

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
    if (!ensure(MenuClass != nullptr)) return;

    UMainMenu* Menu = CreateWidget<UMainMenu>(this, MenuClass);
    if (!ensure(Menu != nullptr)) return;

    Menu->AddToViewport();

    APlayerController* PlayerController = GetFirstLocalPlayerController();
    if (!ensure(PlayerController != nullptr)) return;

    FInputModeUIOnly InputModeData;
    InputModeData.SetWidgetToFocus(Menu->TakeWidget());
    InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    PlayerController->SetInputMode(InputModeData);
    PlayerController->bShowMouseCursor = true;

    Menu->SetMenuInterface(this);
}

void UPoolGameInstance::Host()
{
    UEngine* Engine = GetEngine();
    if (!ensure(Engine != nullptr)) return;

    Engine->AddOnScreenDebugMessage(0, 2, FColor::Green, TEXT("Hosting"));

    UWorld* World = GetWorld();
    if (!ensure(World != nullptr)) return;

    World->ServerTravel("/Game/Maps/Pool_cozy_room?listen");
}

void UPoolGameInstance::Join(const FString& Address)
{
    UEngine* Engine = GetEngine();
    if (!ensure(Engine != nullptr)) return;

    Engine->AddOnScreenDebugMessage(0, 5, FColor::Green, FString::Printf(TEXT("Joining %s"), *Address));

    APlayerController* PlayerController = GetFirstLocalPlayerController();
    if (!ensure(PlayerController != nullptr)) return;

    PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
}
