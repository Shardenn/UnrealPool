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
    if (m_pControlledBilliardist)
    {
        auto CurrentState = m_pControlledBilliardist->GetState();

        switch (CurrentState)
        {
            case FBilliardistState::WALKING:
            {
                if (m_pPlayerSpline)
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
                break;
            }
            case FBilliardistState::PICKING:
            {
                // allow only camera controls
                // small crosshair for ball selecting is visible
                // on LBM we pick a ball and goto aiming state
                break;
            }
            case FBilliardistState::AIMING:
            {
                // we follow only the selected ball - camera flies around it
                // LBM down -> we start gaining/losing the strength meter
                // LBM up -> hit occurs, we go to the observing state
                break;
            }
            case FBilliardistState::OBSERVING:
            {
                // we are allowed to switch between different cameras aroung the whole room
                // we are not allowed to move or freely control the camera generally
                // if the player selects the camera in his own playable charater, then he can move, but he only observes
                break;
            }
            case FBilliardistState::EXAMINING:
            {
                break;
            }
        }
    }
}

void ABilliardistController::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // Replicate to everyone
    DOREPLIFETIME(ABilliardistController, m_pPlayerSpline);
    DOREPLIFETIME(ABilliardistController, m_pSelectedBall);
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
    //GetPawn()->SetActorLocation(NewLocation);
    Multicast_MovePlayer(NewLocation);
}

bool ABilliardistController::Multicast_MovePlayer_Validate(FVector NewLocation) { return true; }

void ABilliardistController::Multicast_MovePlayer_Implementation(FVector NewLocation)
{
    GetPawn()->SetActorLocation(NewLocation);
}

void ABilliardistController::SetBall(ABall* NewBall)
{
    Server_SetBall(NewBall);
}

bool ABilliardistController::Server_SetBall_Validate(ABall*) { return true; }

void ABilliardistController::Server_SetBall_Implementation(ABall* NewBall)
{
    m_pSelectedBall = NewBall;
}