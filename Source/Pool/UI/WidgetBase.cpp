// Copyright 2019 Andrei Vikarchuk.


#include "WidgetBase.h"

bool UWidgetBase::Initialize()
{
    bool Success = Super::Initialize();
    if (!Success) return false;

    bIsFocusable = true;
    return true;
}

void UWidgetBase::Setup()
{
    AddToViewport();

    APlayerController* PlayerController = GetOwningPlayer();
    if (!ensure(PlayerController != nullptr)) return;

    FInputModeUIOnly InputModeData;
    InputModeData.SetWidgetToFocus(TakeWidget());
    InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    PlayerController->SetInputMode(InputModeData);
    PlayerController->bShowMouseCursor = true;
}

void UWidgetBase::Teardown()
{
    APlayerController* PlayerController = GetOwningPlayer();
    if (!ensure(PlayerController != nullptr)) return;

    FInputModeGameOnly InputModeData;
    PlayerController->SetInputMode(InputModeData);
    PlayerController->bShowMouseCursor = false;

    RemoveFromViewport();
}