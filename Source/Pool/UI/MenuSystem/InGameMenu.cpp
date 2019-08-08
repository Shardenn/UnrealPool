// Copyright 2019 Andrei Vikarchuk.


#include "InGameMenu.h"
#include "Components/Button.h"

bool UInGameMenu::Initialize()
{
    bool Success = Super::Initialize();
    if (!Success) return false;

    if (!ensure(ButtonCancel != nullptr)) return false;
    ButtonCancel->OnClicked.AddDynamic(this, &UInGameMenu::HideInGameMenu);

    if (!ensure(ButtonQuit != nullptr)) return false;
    ButtonQuit->OnClicked.AddDynamic(this, &UInGameMenu::QuitPressed);

    return true;
}

void UInGameMenu::HideInGameMenu()
{
    Teardown();
}

void UInGameMenu::QuitPressed()
{
    Teardown();
    if (MenuInterface)
        MenuInterface->LoadMainMenuLevel();
}
