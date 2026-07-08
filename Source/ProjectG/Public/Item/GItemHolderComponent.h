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

    bool HasEquipment() const { return EquipmentItem != nullptr; }

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

    UPROPERTY(ReplicatedUsing = OnRep_EquipmentItem, VisibleAnywhere, BlueprintReadOnly, Category = "Item")
    TObjectPtr<UGConsumableDefinition> EquipmentItem;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
    TObjectPtr<UStaticMeshComponent> EquipmentMeshComponent;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FName EquipmentSocketName = TEXT("HandGrip_R");

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
    TObjectPtr<class UNiagaraComponent> BladeFXComponent;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    TObjectPtr<class UNiagaraSystem> BladeFX;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
    TObjectPtr<class UNiagaraComponent> ShaftFXComponent;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    TObjectPtr<class UNiagaraSystem> ShaftFX;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FVector BladeFXScale = FVector(0.3f, 0.3f, 0.3f);

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FVector ShaftFXScale = FVector(0.3f, 0.3f, 0.3f);

    UFUNCTION()
    void OnRep_ConsumableItem();

    UFUNCTION()
    void OnRep_EquipmentItem();

private:
    void RefreshConsumableMesh();

    void RefreshEquipmentMesh();
};