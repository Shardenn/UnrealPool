// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ServerRow.generated.h"

/**
 *
 */
UCLASS()
class POOL_API UServerRow : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* TextServerName;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* TextHostUsername;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* TextConnectionFraction;

    UPROPERTY(BlueprintReadOnly)
    bool bSelected = false;

    void Setup(class UMainMenu* Parent, uint32 Index);

private:
    UPROPERTY(meta = (BindWidget))
    class UButton* ButtonSelectServer;

    class UMainMenu* Parent;
    uint32 ServerIndex;

    UFUNCTION()
    void OnRowClicked();
};
