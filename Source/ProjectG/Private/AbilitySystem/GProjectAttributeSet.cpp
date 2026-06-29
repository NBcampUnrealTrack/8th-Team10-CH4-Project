// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/GProjectAttributeSet.h"

#include "GameplayEffectExtension.h"

UGProjectAttributeSet::UGProjectAttributeSet()
{
	InitHealth(100.0f);
	InitMaxHealth(100.0f);
	InitSP(100.0f);
	InitMaxSP(100.0f);
	InitMoveSpeed(600.0f);
	InitAttackPower(10.0f);
	InitDefense(0.0f);
	InitKnockbackPower(1.0f);
	InitKnockbackResistance(0.0f);
}

void UGProjectAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	ClampAttribute(Attribute, NewValue);
}

void UGProjectAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
	}
	else if (Data.EvaluatedData.Attribute == GetSPAttribute())
	{
		SetSP(FMath::Clamp(GetSP(), 0.0f, GetMaxSP()));
	}
}

void UGProjectAttributeSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	else if (Attribute == GetMaxHealthAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f);
	}
	else if (Attribute == GetSPAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxSP());
	}
	else if (Attribute == GetMaxSPAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
	else if (Attribute == GetMoveSpeedAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
}

#undef ATTRIBUTE_ACCESSORS
