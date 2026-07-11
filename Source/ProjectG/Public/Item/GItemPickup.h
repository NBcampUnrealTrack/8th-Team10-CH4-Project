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

    void ResetForNewRound();

    bool IsPickupAvailable() const { return bPickupAvailable; }


protected:
    virtual void BeginPlay() override;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    void SetPickupEnabled(bool bEnabled);

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

private:
    void SetPickupAvailable(bool bAvailable);
    void ApplyPickupAvailable();

    UFUNCTION()
    void OnRep_PickupAvailable();

    UPROPERTY(ReplicatedUsing = OnRep_PickupAvailable)
    bool bPickupAvailable = true;

    FTransform InitialTransform;
};