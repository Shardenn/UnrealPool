// Fill out your copyright notice in the Description page of Project Settings.

#include "BilliardistController.h"
#include "Pool.h"
#include "UnrealNetwork.h"

ABilliardistController::ABilliardistController()
{
    
}

void ABilliardistController::BeginPlay()
{
    SelfInitializePawn();
}

void ABilliardistController::Tick(float DeltaTime)
{
    // direction that the pawn would go in case we do not have a spline path
    auto Direction{ FVector::ZeroVector };

    if (m_pControlledBilliardist && 
        m_pPlayerSpline)
    {
        Direction = m_pControlledBilliardist->m_fCurrentMoveDirection;

        if (Direction != FVector::ZeroVector)
        {
            auto SplineTangent = m_pPlayerSpline->GetDirectionAtDistanceAlongSpline(m_fDistanceAlongSpline, ESplineCoordinateSpace::World);
            float cosin = cosin = FVector::DotProduct(SplineTangent, Direction) /
                (SplineTangent.Size() * Direction.Size()); // cos between spline tangent and move direction without spline
            m_fDistanceAlongSpline += cosin * DeltaTime * m_pControlledBilliardist->GetMoveSpeed();

            if (m_fDistanceAlongSpline >= m_pPlayerSpline->GetSplineLength())
                m_fDistanceAlongSpline -= m_pPlayerSpline->GetSplineLength();
            else if (m_fDistanceAlongSpline < 0)
                m_fDistanceAlongSpline += m_pPlayerSpline->GetSplineLength();

            Server_MovePlayer(m_pPlayerSpline->GetLocationAtDistanceAlongSpline(m_fDistanceAlongSpline,
                ESplineCoordinateSpace::World));
        }
        m_pControlledBilliardist->m_fCurrentMoveDirection = FVector::ZeroVector;
    }
}

void ABilliardistController::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // Replicate to everyone
    DOREPLIFETIME(ABilliardistController, m_pPlayerSpline);
}

void ABilliardistController::SelfInitializePawn()
{
    auto PossessedPawn = Cast<ABilliardist>(GetPawn());
    if (!PossessedPawn)
    {
        UE_LOG(LogPool, Error, TEXT("%s could not cast its possessed pawn to the billiardist in SelfInitializePawn."), *GetName());
    }
    else
    {
        m_pControlledBilliardist = PossessedPawn;
        m_pPlayerSpline = m_pControlledBilliardist->GetSpline();
        if (!m_pPlayerSpline)
        {
            UE_LOG(LogPool, Error, TEXT("%s has proper possessed billiardist %s but could not retrieve proper spline from it in SelfInitializePawn."), 
                *GetName(),
                *m_pControlledBilliardist->GetName());
        }
        else
        {
            UE_LOG(LogPool, Log, TEXT("%s successfully initialized %s as its Billiardist pawn and %s as a spline"), *GetName(),
                *m_pControlledBilliardist->GetName(),
                *m_pPlayerSpline->GetName());
        }
    }
}

bool ABilliardistController::Server_MovePlayer_Validate(FVector)
{
    return true;
}

void ABilliardistController::Server_MovePlayer_Implementation(FVector NewLocation)
{
    GetPawn()->SetActorLocation(NewLocation);
}