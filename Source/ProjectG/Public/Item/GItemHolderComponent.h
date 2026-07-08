#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GItemHolderComponent.generated.h"

class UGConsumableDefinition;
class UStaticMeshComponent;
class UNiagaraSystem;
class UNiagaraComponent;

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

    bool HasConsumable() const { return ConsumableItem != nullptr; }

    UFUNCTION(BlueprintCallable, Category = "Item")
    bool TryPickupNearby();

    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_PlayUseEffect(UNiagaraSystem* Effect);

protected:
    virtual void BeginPlay() override;

    UPROPERTY(ReplicatedUsing = OnRep_ConsumableItem, VisibleAnywhere, BlueprintReadOnly, Category = "Item")
    TObjectPtr<UGConsumableDefinition> ConsumableItem;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
    TObjectPtr<UStaticMeshComponent> ConsumableMeshComponent;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FName ConsumableSocketName = TEXT("HandGrip_L");

    UFUNCTION()
    void OnRep_ConsumableItem();

private:
    void RefreshConsumableMesh();

};