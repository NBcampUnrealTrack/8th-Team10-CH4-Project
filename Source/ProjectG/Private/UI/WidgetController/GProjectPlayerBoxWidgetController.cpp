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
	OnPlayerNameChanged.Broadcast(GetPlayerName());
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

	if (AGProjectPlayerState* PS = Cast<AGProjectPlayerState>(PlayerState))
	{
		PS->OnPlayerNameChanged.RemoveAll(this);
		PS->OnPlayerNameChanged.AddUObject(this, &ThisClass::PlayerNameChanged);
	}
}

FText UGProjectPlayerBoxWidgetController::GetPlayerName() const
{
	AGProjectPlayerState* PS = Cast<AGProjectPlayerState>(PlayerState);
	return  PS ? FText::FromString(PS->GetPlayerName()) : FText::GetEmpty();
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

void UGProjectPlayerBoxWidgetController::PlayerNameChanged(const FString& NewName)
{
	OnPlayerNameChanged.Broadcast(FText::FromString(NewName));
}
