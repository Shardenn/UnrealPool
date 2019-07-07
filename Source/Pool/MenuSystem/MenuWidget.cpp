// Fill out your copyright notice in the Description page of Project Settings.


#include "MenuWidget.h"

bool UMenuWidget::Initialize()
{
    bool Success = Super::Initialize();
    if (!Success) return false;

    bIsFocusable = true;
    return true;
}

void UMenuWidget::SetMenuInterface(IMenuInterface* NewInterface)
{
    this->MenuInterface = NewInterface;
}

void UMenuWidget::Setup()
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

void UMenuWidget::Teardown()
{
    APlayerController* PlayerController = GetOwningPlayer();
    if (!ensure(PlayerController != nullptr)) return;

    FInputModeGameOnly InputModeData;
    PlayerController->SetInputMode(InputModeData);
    PlayerController->bShowMouseCursor = false;

    RemoveFromViewport();
}
