// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Item/GProjectItemDefinition.h"
#include "GProjectConsumableDefinition.generated.h"

class UGameplayEffect;
class UNiagaraSystem;
class USoundBase;

UCLASS(BlueprintType)
class PROJECTG_API UGProjectConsumableDefinition : public UGProjectItemDefinition
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consumable")
	TArray<TSubclassOf<UGameplayEffect>> EffectsToApply;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consumable", meta = (ClampMin = "1.0"))
	float EffectLevel = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consumable|Feedback")
	TObjectPtr<UNiagaraSystem> UseEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consumable|Feedback")
	TObjectPtr<USoundBase> UseSound;
};
