// Copyright 2019 Andrei Vikarchuk.


#include "PocketArea.h"

#include "GameplayLogic/TurnBasedGameState.h"

#include "Kismet/GameplayStatics.h"

void UPocketArea::OnHoverOver_Implementation()
{
    UE_LOG(LogTemp, Warning, TEXT("UPocketArea::OnHoverOver not implemented in BP"));
}

void UPocketArea::OnStopHoverOver_Implementation()
{
    UE_LOG(LogTemp, Warning, TEXT("UPocketArea::OnStopHoverOver not implemented in BP"));
}

void UPocketArea::OnSelected_Implementation()
{
    UE_LOG(LogTemp, Warning, TEXT("UPocketArea::OnSelected not implemented in BP"));
}

void UPocketArea::OnDeselected_Implementation()
{
    UE_LOG(LogTemp, Warning, TEXT("UPocketArea::OnDeselected not implemented in BP"));
}

void UPocketArea::BeginPlay()
{
    Super::BeginPlay();

    const auto GS = Cast<ATurnBasedGameState>(UGameplayStatics::GetGameState(GetWorld()));
    if (!ensure(GS != nullptr)) return;

    GS->OnTurnEnd.AddDynamic(this, &UPocketArea::OnTurnEndFired);
}

void UPocketArea::OnTurnEndFired()
{
    OnStopHoverOver();
    OnDeselected();
}
