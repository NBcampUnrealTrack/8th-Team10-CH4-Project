// GConsumableDefinition.h
#pragma once

#include "CoreMinimal.h"
#include "Item/GItemDefinition.h"
#include "GConsumableDefinition.generated.h"

class UGameplayEffect;
class UStaticMesh;
class UNiagaraSystem;

UCLASS()
class PROJECTG_API UGConsumableDefinition : public UGItemDefinition
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consumable")
    TArray<TSubclassOf<UGameplayEffect>> EffectsToApply;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consumable")
    float EffectLevel = 1.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consumable")
    TObjectPtr<UStaticMesh> HeldMesh;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consumable")
    TObjectPtr<UNiagaraSystem> UseEffect;
};