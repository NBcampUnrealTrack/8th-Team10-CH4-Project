// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/WidgetController/GProjectPlayerBoxWidgetController.h"

#include "AbilitySystem/GProjectAbilitySystemComponent.h"
#include "AbilitySystem/GProjectAttributeSet.h"
#include "GameFramework/PlayerState.h"
#include "Player/GProjectPlayerState.h"
#include "GameplayEffectTypes.h"

void UGProjectPlayerBoxWidgetController::BroadcastInitialValues()
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

void UGProjectPlayerBoxWidgetController::BindCallbacksToDependencies()
{
	UGProjectAbilitySystemComponent* ASC = GetGProjectASC();
	UGProjectAttributeSet* AS = GetGProjectAS();
	if (!ASC || !AS)
	{
		return;
	}

	ASC->GetGameplayAttributeValueChangeDelegate(AS->GetHealthAttribute()).AddUObject(
		this, &ThisClass::HealthChanged);
	ASC->GetGameplayAttributeValueChangeDelegate(AS->GetMaxHealthAttribute()).AddUObject(
		this, &ThisClass::MaxHealthChanged);
	ASC->GetGameplayAttributeValueChangeDelegate(AS->GetSPAttribute()).AddUObject(
		this, &ThisClass::SPChanged);
	ASC->GetGameplayAttributeValueChangeDelegate(AS->GetMaxSPAttribute()).AddUObject(
		this, &ThisClass::MaxSPChanged);
}

FText UGProjectPlayerBoxWidgetController::GetPlayerName() const
{
	if (PlayerState)
	{
		AGProjectPlayerState* PS = Cast<AGProjectPlayerState>(PlayerState);
		if (PS && !PS->GetPlayerName().IsEmpty())
		{
			return FText::FromString(PS->GetPlayerName());
		}
		return FText::FromString(PlayerState->GetPlayerName());
	}
	return FText::GetEmpty();
}

void UGProjectPlayerBoxWidgetController::HealthChanged(const FOnAttributeChangeData& Data)
{
	OnHealthChanged.Broadcast(Data.NewValue);
}

void UGProjectPlayerBoxWidgetController::MaxHealthChanged(const FOnAttributeChangeData& Data)
{
	OnMaxHealthChanged.Broadcast(Data.NewValue);
}

void UGProjectPlayerBoxWidgetController::SPChanged(const FOnAttributeChangeData& Data)
{
	OnSPChanged.Broadcast(Data.NewValue);
}

void UGProjectPlayerBoxWidgetController::MaxSPChanged(const FOnAttributeChangeData& Data)
{
	OnMaxSPChanged.Broadcast(Data.NewValue);
}
