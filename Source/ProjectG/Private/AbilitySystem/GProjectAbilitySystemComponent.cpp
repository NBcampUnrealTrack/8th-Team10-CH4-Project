// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/GProjectAbilitySystemComponent.h"

#include "AbilitySystem/Abilities/GProjectGameplayAbility.h"

void UGProjectAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid())
	{
		return;
	}

	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		const UGProjectGameplayAbility* ProjectAbility = Cast<UGProjectGameplayAbility>(AbilitySpec.Ability);
		if (!ProjectAbility)
		{
			continue;
		}

		if (ProjectAbility->StartupInputTag.MatchesTagExact(InputTag))
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
		const UGProjectGameplayAbility* ProjectAbility = Cast<UGProjectGameplayAbility>(AbilitySpec.Ability);
		if (!ProjectAbility)
		{
			continue;
		}

		if (ProjectAbility->StartupInputTag.MatchesTagExact(InputTag))
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
		const UGProjectGameplayAbility* ProjectAbility = Cast<UGProjectGameplayAbility>(AbilitySpec.Ability);
		if (!ProjectAbility)
		{
			continue;
		}

		if (!ProjectAbility->StartupInputTag.MatchesTagExact(InputTag))
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
