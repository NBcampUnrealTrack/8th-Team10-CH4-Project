#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GItemPickup.generated.h"

class UGConsumableDefinition;
class UStaticMeshComponent;
class USphereComponent;

UCLASS()
class PROJECTG_API AGItemPickup : public AActor
{
    GENERATED_BODY()

public:
    AGItemPickup();

    bool TryPickup(AActor* Picker);

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup")
    TObjectPtr<UGConsumableDefinition> ItemDefinition;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup")
    TObjectPtr<UStaticMeshComponent> MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup")
    TObjectPtr<USphereComponent> OverlapSphere;

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    UPROPERTY()
    TObjectPtr<AActor> OverlappingActor;
};