// Copyright Epic Games, Inc. All Rights Reserved.

#include "Input/GProjectInputConfig.h"

#include "InputAction.h"

const UInputAction* UGProjectInputConfig::FindAbilityInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound) const
{
	for (const FGProjectInputAction& Action : AbilityInputActions)
	{
		if (Action.InputAction && Action.InputTag.MatchesTagExact(InputTag))
		{
			return Action.InputAction;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(LogTemp, Error, TEXT("Input action not found for input tag [%s] on input config [%s]."), *InputTag.ToString(), *GetNameSafe(this));
	}

	return nullptr;
}
