#include "MainMenu.h"
#include "Pool.h"
#include "ServerRow.h"

#include "UObject/ConstructorHelpers.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "Components/EditableText.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "Components/EditableTextBox.h"

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
    ButtonHost->OnClicked.AddDynamic(this, &UMainMenu::OpenHostMenu);

    if (!ensure(ButtonHostConfirm != nullptr)) return false;
    ButtonHostConfirm->OnClicked.AddDynamic(this, &UMainMenu::HostServer);

    if (!ensure(ButtonHostCancel != nullptr)) return false;
    ButtonHostCancel->OnClicked.AddDynamic(this, &UMainMenu::OpenMainMenu);

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
        FString ServerName = TextBoxServerHostName->GetText().ToString();
        if (!ServerName.IsEmpty())
            MenuInterface->Host(ServerName);
    }
}

void UMainMenu::SetServerList(TArray<FServerData> ServerDatas)
{
    ServerList->ClearChildren();

    uint32 i = 0;
    for (const auto& ServerData : ServerDatas)
    {
        UServerRow* ServerRow = CreateWidget<UServerRow>(this, ServerRowClass);
        if (!ensure(ServerRow != nullptr)) continue;

        ServerRow->TextServerName->SetText(FText::FromString(ServerData.Name));
        ServerRow->TextHostUsername->SetText(FText::FromString(ServerData.HostUsername));
        FString FractionText = FString::Printf(TEXT("%d/%d"), ServerData.CurrentPlayers, ServerData.MaxPlayers);
        ServerRow->TextConnectionFraction->SetText(FText::FromString(FractionText));
        ServerRow->Setup(this, i++);

        ServerList->AddChild(ServerRow);
    }
}

void UMainMenu::SelectIndex(uint32 Index)
{
    SelectedServerIndex = Index;
    UpdateChildren();
}

void UMainMenu::UpdateChildren()
{
    for (int32 i = 0; i < ServerList->GetChildrenCount(); i++)
    {
        auto Row = Cast<UServerRow>(ServerList->GetChildAt(i));
        if (Row)
        {
            Row->bSelected = (SelectedServerIndex.IsSet() && 
                SelectedServerIndex.GetValue() == i);
        }
    }
}

void UMainMenu::JoinServer()
{
    if (SelectedServerIndex.IsSet() && MenuInterface != nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("Selected index is %d"), SelectedServerIndex.GetValue());
        MenuInterface->Join(SelectedServerIndex.GetValue());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Selected index is not set"));
    }
}

void UMainMenu::OpenMainMenu()
{
    if (!ensure(MenuSwitcher != nullptr)) return;
    if (!ensure(MainMenu != nullptr)) return;
    MenuSwitcher->SetActiveWidget(MainMenu);
}

void UMainMenu::OpenHostMenu()
{
    if (!ensure(MenuSwitcher != nullptr)) return;
    if (!ensure(HostMenu != nullptr)) return;
    MenuSwitcher->SetActiveWidget(HostMenu);
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