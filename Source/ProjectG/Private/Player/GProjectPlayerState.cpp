// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/GProjectPlayerState.h"

#include "AbilitySystem/GProjectAbilitySystemComponent.h"
#include "AbilitySystem/GProjectAttributeSet.h"

AGProjectPlayerState::AGProjectPlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<UGProjectAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<UGProjectAttributeSet>(TEXT("AttributeSet"));

	SetNetUpdateFrequency(100.0f);
}

UAbilitySystemComponent* AGProjectPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UGProjectAbilitySystemComponent* AGProjectPlayerState::GetGProjectAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UGProjectAttributeSet* AGProjectPlayerState::GetAttributeSet() const
{
	return AttributeSet;
}
