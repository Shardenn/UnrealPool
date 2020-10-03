// Copyright 2019 Andrei Vikarchuk.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GSWithNamedShot.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UGSWithNamedShot : public UInterface
{
    GENERATED_BODY()
};

class UPocketArea;
class ABall;

USTRUCT(BlueprintType)
struct FNamedShot
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UPocketArea* SelectedPocket;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ABall* SelectedBall;

    FNamedShot()
    {
        SelectedPocket = nullptr;
        SelectedBall = nullptr;
    }

    FNamedShot(UPocketArea* Pocket, ABall* Ball)
    {
        SelectedPocket = Pocket;
        SelectedBall = Ball;
    }
    /*
    bool operator== (const FNamedShot& a)
    {
        if (a.SelectedBall == this->SelectedBall &&
            a.SelectedPocket == this->SelectedPocket)
            return true;
        return false;
    }

    bool operator!= (const FNamedShot& a)
    {
        return !(*this == a);
    }
    */
    bool IsSet()
    {
        if (SelectedBall == nullptr ||
            SelectedPocket == nullptr)
            return false;
        return true;
    }

    bool IsEqual(const FNamedShot& other)
    {
        if (other.SelectedBall == this->SelectedBall &&
            other.SelectedPocket == this->SelectedPocket)
            return true;
        return false;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNamedShotRegistered, FNamedShot, NamedShot);

/**
 * An interface to be inherited by GameState which supports a named shot rule
 */
class POOL_API IGSWithNamedShot
{
    GENERATED_BODY()

public:
    virtual void RegisterNamedShot(UPocketArea* SelectedPocket, ABall* SelectedBall) = 0;
    virtual void RegisterPredictedShot(UPocketArea* SelectedPocket, ABall* SelectedBall) = 0;
protected:
    // For replication functions purposes
    virtual void RegisterNamedShot_Internal(UPocketArea* SelectedPocket, ABall* SelectedBall);
    virtual void RegisterPredictedShot_Internal(UPocketArea* SelectedPocket, ABall* SelectedBall);
};
