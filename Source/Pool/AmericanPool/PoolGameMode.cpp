#include "PoolGameMode.h"

#include "Objects/Table/Table.h"
#include "Billiardist.h"
#include "PoolGameState.h"

#include "EngineUtils.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/SplineComponent.h"

bool APoolGameMode::ReadyToStartMatch_Implementation()
{
    APoolGameState* GameState = GetGameState<APoolGameState>();
    if (!ensure(GameState != nullptr)) return false;

    if (GameState->PlayersReadyNum == RequiredPlayersReadyNum)
    {
        return true;
    }

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

void APoolGameMode::HandleMatchHasStarted()
{
    Super::HandleMatchHasStarted();

    auto Balls = GameTable->SpawnBalls();

    auto PoolGameState = GetGameState<APoolGameState>();
    PoolGameState->ActiveBalls = Balls;

    PoolGameState->SwitchTurn();
}

void APoolGameMode::HandleMatchHasEnded()
{
    Super::HandleMatchHasEnded();

    UE_LOG(LogGameMode, Warning, TEXT("Match has ended"));
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