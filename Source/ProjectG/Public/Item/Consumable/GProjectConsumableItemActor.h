// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Item/GProjectItemActorBase.h"
#include "GProjectConsumableItemActor.generated.h"

class UGameplayEffect;
class UGProjectConsumableDefinition;
class UNiagaraSystem;
class USoundBase;
class UAnimMontage;

UCLASS()
class PROJECTG_API AGProjectConsumableItemActor : public AGProjectItemActorBase
{
	GENERATED_BODY()

public:
	virtual bool CanBePickedUpBy(const AGProjectCharacter* Character) const override;
	virtual bool CanUse(const AGProjectCharacter* Character) const override;
	virtual bool Use_Implementation(AGProjectCharacter* Character) override;
	virtual UAnimMontage* GetUseMontage() const override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consumable")
	TArray<TSubclassOf<UGameplayEffect>> EffectsToApply;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consumable", meta = (ClampMin = "1.0"))
	float EffectLevel = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consumable|Feedback")
	TObjectPtr<UNiagaraSystem> UseEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consumable|Feedback")
	TObjectPtr<USoundBase> UseSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consumable")
	TObjectPtr<UAnimMontage> UseMontage;

private:
	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayUseFeedback(UNiagaraSystem* InUseEffect, USoundBase* InUseSound, FVector Location, FRotator Rotation);

	const UGProjectConsumableDefinition* GetConsumableDefinition() const;
};
