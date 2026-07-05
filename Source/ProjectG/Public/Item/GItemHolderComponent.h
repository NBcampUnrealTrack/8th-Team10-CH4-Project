#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GItemHolderComponent.generated.h"

class UGConsumableDefinition;
class UStaticMeshComponent;
class UNiagaraSystem;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECTG_API UGItemHolderComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UGItemHolderComponent();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION(BlueprintCallable, Category = "Item")
    void HoldItem(UGConsumableDefinition* NewItem);

    UFUNCTION(BlueprintCallable, Category = "Item")
    void UseHeldItem();

    UFUNCTION(BlueprintCallable, Category = "Item")
    bool HasHeldItem() const { return HeldItem != nullptr; }

    UFUNCTION(BlueprintCallable, Category = "Item")
    bool TryPickupNearby();

    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_PlayUseEffect(UNiagaraSystem* Effect);

protected:
    virtual void BeginPlay() override;

    UPROPERTY(ReplicatedUsing = OnRep_HeldItem, VisibleAnywhere, BlueprintReadOnly, Category = "Item")
    TObjectPtr<UGConsumableDefinition> HeldItem;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
    TObjectPtr<UStaticMeshComponent> HeldMeshComponent;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FName AttachSocketName = TEXT("hand_r");

    UFUNCTION()
    void OnRep_HeldItem();

private:
    void RefreshHeldMesh();
};