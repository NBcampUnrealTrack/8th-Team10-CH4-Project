// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Abilities/GProjectPickupItemAbility.h"

#include "Character/GProjectCharacter.h"
#include "Item/GProjectItemHolderComponent.h"

void UGProjectPickupItemAbility::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (AGProjectCharacter* Character = Cast<AGProjectCharacter>(GetAvatarActorFromActorInfo()))
	{
		if (UGProjectItemHolderComponent* ItemHolder = Character->GetItemHolderComponent())
		{
			ItemHolder->TryPickupNearby();
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
