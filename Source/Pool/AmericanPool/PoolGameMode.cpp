#include "PoolGameMode.h"

#include "Table.h"
#include "Billiardist.h"

#include "EngineUtils.h"
#include "Kismet/KismetMathLibrary.h"

bool APoolGameMode::ReadyToStartMatch_Implementation()
{
    return false;
}

void APoolGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
    PlayerControllers.Add(NewPlayer);
    RestartPlayer(NewPlayer);
}

void APoolGameMode::RestartPlayer(AController* Controller)
{
    if (GameTable == nullptr)
    {
        bool bSuccess = InitializeTable();
        if (!bSuccess)
        {
            UE_LOG(LogGameMode, Warning, TEXT("Did not find a game table"));
            return;
        }
    }

    FActorSpawnParameters SpawnInfo;
    SpawnInfo.ObjectFlags |= RF_Transient;

    UWorld* World = GetWorld();
    auto Pawn = World->SpawnActor<APawn>(DefaultPawnClass, GetSpawnTransform());
    
    Cast<ABilliardist>(Pawn)->Initialize(GameTable->GetSplinePath());

    Controller->Possess(Pawn);

    Controller->ClientSetRotation(Controller->GetPawn()->GetActorRotation(), true);
}

bool APoolGameMode::InitializeTable()
{
    UWorld* World = GetWorld();
    for (TActorIterator<ATable> It(World); It; ++It)
    {
        ATable* Table = *It;
        if (Table)
        {
            this->GameTable = Table;
            return true;
        }
    }
    return false;
}

FTransform APoolGameMode::GetSpawnTransform()
{
    USplineComponent* TableSpline = GameTable->GetSplinePath();
    
    FVector Location = TableSpline->GetLocationAtDistanceAlongSpline(0,
        ESplineCoordinateSpace::World);
    FRotator Rotation = UKismetMathLibrary::FindLookAtRotation(Location, GameTable->GetActorLocation());
    Rotation.Pitch = 0;
    Rotation.Roll = 0;
    
    FTransform SpawnTransform;
    SpawnTransform.SetLocation(Location);
    SpawnTransform.SetRotation(FQuat(Rotation));
    return SpawnTransform;
}