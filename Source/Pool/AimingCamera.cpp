#include "AimingCamera.h"
#include "BilliardistController.h"

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

}

void AAimingCamera::ReturnPressHandle()
{
    auto controller = Cast<ABilliardistController>(GetController());
    check(controller != nullptr);

}


