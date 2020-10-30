// Copyright 2019 Andrei Vikarchuk.


#include "ReplayController.h"
#include "Engine/DemoNetDriver.h"

AReplayController::AReplayController(const FObjectInitializer& Initializer) : 
    Super(Initializer)
{
    bShowMouseCursor = true;
    PrimaryActorTick.bTickEvenWhenPaused = true;
    bShouldPerformFullTickWhenPaused = true;
}

bool AReplayController::SetCurrentReplayPauseState(bool bDoPause)
{
    static const auto CVarAA = IConsoleManager::Get().
                    FindConsoleVariable(TEXT("r.DefaultFeature.AntiAliasing"));
    static const auto CVarMB = IConsoleManager::Get().
                    FindConsoleVariable(TEXT("r.DefaultFeature.MotionBlur"));

    AWorldSettings* WorldSettings = GetWorldSettings();
    
    if (bDoPause)
    {
        PreviousAASetting = CVarAA->GetInt();
        PreviousMBSetting = CVarMB->GetInt();

        CVarAA->Set(1);
        CVarMB->Set(0);

        WorldSettings->SetPauserPlayerState(PlayerState);
        return true;
    }

    CVarAA->Set(PreviousAASetting);
    CVarMB->Set(PreviousMBSetting);

    WorldSettings->SetPauserPlayerState(NULL);
    return false;
}

float AReplayController::GetCurrentReplayTotalTimeInSec() const
{
    if (GetWorld())
    {
        if (GetWorld()->DemoNetDriver)
        {
            return GetWorld()->DemoNetDriver->DemoTotalTime;
        }
    }
    return 0.f;
}

float AReplayController::GetCurrentReplayCurrentTimeInSec() const
{
    if (GetWorld())
    {
        if (GetWorld()->DemoNetDriver)
        {
            return GetWorld()->DemoNetDriver->DemoCurrentTime;
        }
    }
    return 0.f;
}

void AReplayController::SetCurrentReplayCurrentTimeToSec(float Seconds)
{
    if (GetWorld())
    {
        if (GetWorld()->DemoNetDriver)
        {
            GetWorld()->DemoNetDriver->GotoTimeInSeconds(Seconds);
        }
    }
}

void AReplayController::SetCurrentReplayPlayRate(float PlayRate)
{
     if (GetWorld())
     {
         if (GetWorld()->DemoNetDriver)
         {
             GetWorld()->GetWorldSettings()->DemoPlayTimeDilation = PlayRate;
         }
     } 
}
