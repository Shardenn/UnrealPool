#include "MainMenu.h"
#include "Pool.h"
#include "ServerRow.h"

#include "UObject/ConstructorHelpers.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "Components/EditableText.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"

UMainMenu::UMainMenu(const FObjectInitializer& ObjectInitializer)
{
    ConstructorHelpers::FClassFinder<UUserWidget> ServerRowBPClass(TEXT("/Game/UI/MenuSystem/WBP_ServerRow"));
    if (!ensure(ServerRowBPClass.Class != nullptr)) return;

    ServerRowClass = ServerRowBPClass.Class;
}

bool UMainMenu::Initialize()
{
    bool Success = Super::Initialize();
    if (!Success) return false;

    if (!ensure(ButtonHost != nullptr)) return false;
    ButtonHost->OnClicked.AddDynamic(this, &UMainMenu::HostServer);

    if (!ensure(ButtonJoinConfirm != nullptr)) return false;
    ButtonJoinConfirm->OnClicked.AddDynamic(this, &UMainMenu::JoinServer);
    
    if (!ensure(ButtonJoin != nullptr)) return false;
    ButtonJoin->OnClicked.AddDynamic(this, &UMainMenu::OpenJoinMenu);

    if (!ensure(ButtonJoinCancel != nullptr)) return false;
    ButtonJoinCancel->OnClicked.AddDynamic(this, &UMainMenu::OpenMainMenu);

    if (!ensure(ButtonQuitGame != nullptr)) return false;
    ButtonQuitGame->OnClicked.AddDynamic(this, &UMainMenu::QuitPressed);

    if (!ensure(ServerList != nullptr)) return false;

    

    return true;
}

void UMainMenu::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
    //Teardown();
}

void UMainMenu::HostServer()
{
    if (MenuInterface != nullptr)
    {
        MenuInterface->Host();
    }
}

void UMainMenu::SetServerList(TArray<FString> ServerNames)
{
    ServerList->ClearChildren();

    for (const FString& ServerName : ServerNames)
    {
        UServerRow* ServerRow = CreateWidget<UServerRow>(this, ServerRowClass);
        if (!ensure(ServerRow != nullptr)) continue;

        ServerRow->ServerName->SetText(FText::FromString(ServerName));

        ServerList->AddChild(ServerRow);
    }
}

void UMainMenu::JoinServer()
{
    if (MenuInterface != nullptr)
    {
        //FString Address = AddressField->GetText().ToString();
        //MenuInterface->Join("");
    }
}

void UMainMenu::OpenMainMenu()
{
    if (!ensure(MenuSwitcher != nullptr)) return;
    if (!ensure(MainMenu != nullptr)) return;
    MenuSwitcher->SetActiveWidget(MainMenu);
}

void UMainMenu::OpenJoinMenu()
{
    if (!ensure(MenuSwitcher != nullptr)) return;
    if (!ensure(JoinMenu != nullptr)) return;
    MenuSwitcher->SetActiveWidget(JoinMenu);

    if (MenuInterface)
    {
        MenuInterface->RequestFindSessions();
    }
}

void UMainMenu::QuitPressed()
{
    APlayerController* PlayerController = GetOwningPlayer();
    if (!ensure(PlayerController != nullptr)) return;

    PlayerController->ConsoleCommand("Quit");
}