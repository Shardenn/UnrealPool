// Copyright 2019 Andrei Vikarchuk.


#include "EndGame.h"
#include "Components/CheckBox.h"

bool UEndGame::Initialize()
{
    bool Success = Super::Initialize();
    if (!Success) return false;

    return true;
}
