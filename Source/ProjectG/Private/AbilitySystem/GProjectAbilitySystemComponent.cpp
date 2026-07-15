// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/GProjectAbilitySystemComponent.h"

#include "AbilitySystem/Abilities/GProjectGameplayAbility.h"
#include "AbilitySystem/Abilities/GProjectPickupAbility.h"
#include "AbilitySystem/Abilities/GProjectUseItemAbility.h"
#include "GProjectGameplayTags.h"

namespace
{
	bool DoesAbilityInputMatch(const UGameplayAbility* Ability, const FGameplayTag& InputTag)
	{
		if (Ability->IsA<UGProjectPickupAbility>())
		{
			return InputTag.MatchesTagExact(GProjectGameplayTags::InputTag_Interaction_Pickup);
		}

		if (Ability->IsA<UGProjectUseItemAbility>())
		{
			return InputTag.MatchesTagExact(GProjectGameplayTags::InputTag_Interaction_Interact);
		}

		const UGProjectGameplayAbility* ProjectAbility = Cast<UGProjectGameplayAbility>(Ability);
		return ProjectAbility && ProjectAbility->StartupInputTag.MatchesTagExact(InputTag);
	}
}

void UGProjectAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid())
	{
		return;
	}

	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (!AbilitySpec.Ability)
		{
			continue;
		}

		if (DoesAbilityInputMatch(AbilitySpec.Ability, InputTag))
		{
			AbilitySpecInputPressed(AbilitySpec);

			if (!AbilitySpec.IsActive())
			{
				TryActivateAbility(AbilitySpec.Handle);
			}
		}
	}
}

void UGProjectAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid())
	{
		return;
	}

	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (!AbilitySpec.Ability)
		{
			continue;
		}

		if (DoesAbilityInputMatch(AbilitySpec.Ability, InputTag))
		{
			AbilitySpecInputReleased(AbilitySpec);
		}
	}
}

void UGProjectAbilitySystemComponent::AbilityInputTagHeld(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid())
	{
		return;
	}

	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (!AbilitySpec.Ability)
		{
			continue;
		}

		if (!DoesAbilityInputMatch(AbilitySpec.Ability, InputTag))
		{
			continue;
		}

		AbilitySpecInputPressed(AbilitySpec);

		if (!AbilitySpec.IsActive())
		{
			TryActivateAbility(AbilitySpec.Handle);
		}
	}
}
