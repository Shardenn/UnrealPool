// Copyright 2019 Andrei Vikarchuk.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "RulesHandler.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class URulesHandler : public UInterface
{
    GENERATED_BODY()
};

// It is common for EACH billiard type. That's just easier and fine
UENUM(BlueprintType)
enum class FFoulReason : uint8
{
    None                    UMETA(DisplayName = "None"),
    CueBallOut              UMETA(DisplayName = "Cue ball out of table"),
    EightBallPocketed       UMETA(DisplayName = "Eight ball pocketed"),
    WrongBallTouchedFirst   UMETA(DisplayName = "Wrong ball touched first"),
    NoCushionHit            UMETA(DisplayName = "No cushion hit"),
    NoBallsHit              UMETA(DisplayName = "No balls hit by the cue")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerFouled, FFoulReason, FoulReason);

/**
 *
 */
class POOL_API IRulesHandler
{
    GENERATED_BODY()

    // Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
    virtual void HandleTurnEnd() = 0;
    virtual void AssignFoul(const FFoulReason Reason) = 0;
    virtual FFoulReason GetFoulReason() = 0;
protected:
    // Override THIS if you want to define turn end implementation
    virtual void HandleTurnEnd_Internal() = 0;
    // Override THIS if you want to define fould assigning implementation
    virtual void AssignFoul_Internal(const FFoulReason Reason) = 0;
};
