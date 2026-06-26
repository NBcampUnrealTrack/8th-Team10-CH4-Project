// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/WidgetController/GProjectOverlayWidgetController.h"

#include "AbilitySystem/GProjectAbilitySystemComponent.h"
#include "AbilitySystem/GProjectAttributeSet.h"
#include "GameplayEffectTypes.h"

void UGProjectOverlayWidgetController::BroadcastInitialValues()
{
	UGProjectAttributeSet* AS = GetGProjectAS();
	if (!AS)
	{
		return;
	}

	OnHealthChanged.Broadcast(AS->GetHealth());
	OnMaxHealthChanged.Broadcast(AS->GetMaxHealth());
	OnSPChanged.Broadcast(AS->GetSP());
	OnMaxSPChanged.Broadcast(AS->GetMaxSP());
}

void UGProjectOverlayWidgetController::BindCallbacksToDependencies()
{
	UGProjectAbilitySystemComponent* ASC = GetGProjectASC();
	UGProjectAttributeSet* AS = GetGProjectAS();
	if (!ASC || !AS)
	{
		return;
	}

	ASC->GetGameplayAttributeValueChangeDelegate(AS->GetHealthAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			OnHealthChanged.Broadcast(Data.NewValue);
		}
	);

	ASC->GetGameplayAttributeValueChangeDelegate(AS->GetMaxHealthAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			OnMaxHealthChanged.Broadcast(Data.NewValue);
		}
	);

	ASC->GetGameplayAttributeValueChangeDelegate(AS->GetSPAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			OnSPChanged.Broadcast(Data.NewValue);
		}
	);

	ASC->GetGameplayAttributeValueChangeDelegate(AS->GetMaxSPAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			OnMaxSPChanged.Broadcast(Data.NewValue);
		}
	);
}
