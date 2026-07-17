// Copyright Epic Games, Inc. All Rights Reserved.

#include "Item/Consumable/GProjectConsumableItemActor.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Character/GProjectCharacter.h"
#include "GameplayEffect.h"
#include "Item/Consumable/GProjectConsumableDefinition.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

bool AGProjectConsumableItemActor::CanBePickedUpBy(const AGProjectCharacter* Character) const
{
	return Super::CanBePickedUpBy(Character);
}

bool AGProjectConsumableItemActor::CanUse(const AGProjectCharacter* Character) const
{
	const UGProjectConsumableDefinition* ConsumableDefinition = GetConsumableDefinition();
	return Character && (EffectsToApply.Num() > 0 || (ConsumableDefinition && ConsumableDefinition->EffectsToApply.Num() > 0));
}

bool AGProjectConsumableItemActor::Use_Implementation(AGProjectCharacter* Character)
{
	if (!Character)
	{
		return false;
	}

	const UGProjectConsumableDefinition* ConsumableDefinition = GetConsumableDefinition();
	const TArray<TSubclassOf<UGameplayEffect>>* ResolvedEffectsToApply = &EffectsToApply;
	float ResolvedEffectLevel = EffectLevel;
	UNiagaraSystem* ResolvedUseEffect = UseEffect;
	USoundBase* ResolvedUseSound = UseSound;

	if (ResolvedEffectsToApply->Num() == 0 && ConsumableDefinition)
	{
		ResolvedEffectsToApply = &ConsumableDefinition->EffectsToApply;
		ResolvedEffectLevel = ConsumableDefinition->EffectLevel;
		ResolvedUseEffect = ConsumableDefinition->UseEffect;
		ResolvedUseSound = ConsumableDefinition->UseSound;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Character);
	if (!ASC)
	{
		return false;
	}

	bool bAppliedAnyEffect = false;
	for (const TSubclassOf<UGameplayEffect>& EffectClass : *ResolvedEffectsToApply)
	{
		if (!EffectClass)
		{
			continue;
		}

		FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
		Context.AddSourceObject(this);

		const FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(
			EffectClass,
			ResolvedEffectLevel,
			Context);
		if (!SpecHandle.IsValid())
		{
			continue;
		}

		ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		bAppliedAnyEffect = true;
	}

	if (!bAppliedAnyEffect)
	{
		return false;
	}

	UseEffect = ResolvedUseEffect;
	UseSound = ResolvedUseSound;

	MulticastPlayUseFeedback(
		Character->GetActorLocation(),
		Character->GetActorRotation());

	return true;
}

void AGProjectConsumableItemActor::MulticastPlayUseFeedback_Implementation(const FVector& Location, const FRotator& Rotation)
{
	if (UseEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			UseEffect,
			Location + FVector(0.0f, 0.0f, 50.0f),
			Rotation);
	}
	if (UseSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			UseSound,
			Location);
	}
}

const UGProjectConsumableDefinition* AGProjectConsumableItemActor::GetConsumableDefinition() const
{
	return Cast<UGProjectConsumableDefinition>(GetItemDefinition());
}

UAnimMontage* AGProjectConsumableItemActor::GetUseMontage() const
{
	if (UseMontage)
	{
		return UseMontage;
	}
	if (const UGProjectConsumableDefinition* Def = GetConsumableDefinition())
	{
		return Def->UseMontage;
	}
	return nullptr;
}
