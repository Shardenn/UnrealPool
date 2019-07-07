#include "MainMenu.h"
#include "Pool.h"

#include "Components/Button.h"

bool UMainMenu::Initialize()
{
    bool Success = Super::Initialize();
    if (!Success) return false;

    bIsFocusable = true;

    if (!ensure(ButtonHost != nullptr)) return false;

    ButtonHost->OnClicked.AddDynamic(this, &UMainMenu::HostServer);
    
    return true;
}

void UMainMenu::SetMenuInterface(IMenuInterface* NewInterface)
{
    this->MenuInterface = NewInterface;
}

void UMainMenu::HostServer()
{
    if (MenuInterface != nullptr)
    {
        MenuInterface->Host();
    }
}
