#include "PoolGameInstance.h"
#include "Pool.h"

#include "OnlineSubsystem.h"

void UPoolGameInstance::Init()
{
    UE_LOG(LogPool, Warning, TEXT("PoolGameInstance::Init()"));

    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
    if (Subsystem != nullptr)
    {
        UE_LOG(LogPool, Warning, TEXT("Found online subsystem named %s"), *Subsystem->GetSubsystemName().ToString());
        auto SessionInterface = Subsystem->GetSessionInterface();
        if (SessionInterface.IsValid())
        {
            UE_LOG(LogPool, Warning, TEXT("Found session interface"));
        }
    }
    else
    {
        UE_LOG(LogPool, Error, TEXT("No online subsystem found"));
    }
}