#include "ServerRow.h"

#include "MainMenu.h"

#include "Components/Button.h"

void UServerRow::Setup(UMainMenu* InParent, uint32 InIndex)
{
    this->Parent = InParent;
    ServerIndex = InIndex;
    ButtonSelectServer->OnClicked.AddDynamic(this, &UServerRow::OnRowClicked);
}

void UServerRow::OnRowClicked()
{
    Parent->SelectIndex(ServerIndex);
}
