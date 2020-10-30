// Copyright 2019 Andrei Vikarchuk.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Cue.generated.h"

UCLASS()
class POOL_API ACue : public AActor
{
    GENERATED_BODY()

public:
    ACue();

    void SubscribeToMeshOverlaps();
    
    uint32 GetMeshOverlapNum() const
    {
        return MeshOverlappingComponents.Num();
    }
    uint32 GetCapsuleOverlapNum() const
    {
        return CapsuleOverlappingComponents.Num();
    }
    
    FVector GetStartCollisionTestPoint() const { return CollisionTestStartingPoint->GetComponentLocation(); }
    FVector GetEndCollisionTestPoint() const { return CollisionTestEndPoint->GetComponentLocation(); }
protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class UStaticMeshComponent* Mesh{ nullptr };
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class UCapsuleComponent* OverlapTestCapsule{ nullptr };
    
    
    virtual void BeginPlay() override;

private:
    UFUNCTION()
    void OnMeshOverlap(UPrimitiveComponent* OverlappedComponent,
            AActor* OtherActor,
            UPrimitiveComponent* OtherComp,
            int32 OtherBodyIndex,
            bool bFromSweep,
            const FHitResult& SweepResult);
    UFUNCTION()
    void OnMeshEndOverlap(UPrimitiveComponent* OverlappedComponent,
            AActor* OtherActor,
            UPrimitiveComponent* OtherComp,
            int32 OtherBodyIndex);

    UFUNCTION()
    void OnCapsuleOverlap(UPrimitiveComponent* OverlappedComponent,
            AActor* OtherActor,
            UPrimitiveComponent* OtherComp,
            int32 OtherBodyIndex,
            bool bFromSweep,
            const FHitResult& SweepResult);
    UFUNCTION()
    void OnCapsuleEndOverlap(UPrimitiveComponent* OverlappedComponent,
            AActor* OtherActor,
            UPrimitiveComponent* OtherComp,
            int32 OtherBodyIndex);

    UPROPERTY(Replicated)
    TArray<class UPrimitiveComponent*> MeshOverlappingComponents;
    UPROPERTY(Replicated)
    TArray<class UPrimitiveComponent*> CapsuleOverlappingComponents;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
    USceneComponent* CollisionTestStartingPoint{ nullptr };
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
    USceneComponent* CollisionTestEndPoint{ nullptr };
};
