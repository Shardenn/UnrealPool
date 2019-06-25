#include "AimingCamera.h"
#include "BilliardistController.h"
#include "Billiardist.h"
#include "Ball.h"
#include "Pool.h"
// Sets default values
AAimingCamera::AAimingCamera()
{
    // Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AAimingCamera::BeginPlay()
{
    Super::BeginPlay();

}

// Called every frame
void AAimingCamera::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    switch (m_eState)
    {
        case FAimingCameraState::InPlayer :
        {
            break;
        }
        case FAimingCameraState::GoingIn :
        {
            auto newLocation = FMath::Lerp(m_vStartingLocation, 
                m_vFinishLocation, 
                m_fAlpha);

            SetActorLocation(newLocation);

            m_fAlpha += DeltaTime * BlendInSpeed;

            if (m_fAlpha >= 1.f)
            {
                m_fAlpha = 0.f;
                SetState(FAimingCameraState::InBall);
            }

            break;
        }
        case FAimingCameraState::InBall :
        {
            break;
        }
        case FAimingCameraState::GoingOut :
        {
            auto newLocation = FMath::Lerp(m_vStartingLocation,
                m_vFinishLocation,
                m_fAlpha);

            SetActorLocation(newLocation);

            m_fAlpha += DeltaTime * BlendOutSpeed;

            if (m_fAlpha >= 1.f)
            {
                m_fAlpha = 0.f;
                SetState(FAimingCameraState::InPlayer);
                auto controller = Cast<ABilliardistController>(GetController());
                check(controller != nullptr);

                auto rot = GetControlRotation();
                controller->SwitchPawn(controller->GetBilliardist());
                controller->SetControlRotation(rot);

                controller->SetState(FBilliardistState::PICKING);
            }

            break;
        }
    }

}

void AAimingCamera::SetState(FAimingCameraState newState)
{
    if (newState == m_eState)
        return;

    m_eState = newState;

    if (m_eState == FAimingCameraState::GoingIn)
    {
        m_vStartingLocation = GetActorLocation() + GetActorForwardVector().GetSafeNormal() *
            m_fSpringArmLength;
        m_vLastPlayerLocation = m_vStartingLocation;
        m_vFinishLocation = m_pSelectedBall->GetActorLocation();
    }
    else if (m_eState == FAimingCameraState::GoingOut)
    {
        m_vFinishLocation = m_vLastPlayerLocation;
        m_vStartingLocation = m_pSelectedBall->GetActorLocation();
    }
}

void AAimingCamera::ReturnPressHandle()
{
    auto controller = Cast<ABilliardistController>(GetController());
    check(controller != nullptr);

}


