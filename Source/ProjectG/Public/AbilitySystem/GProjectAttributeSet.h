// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GProjectAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class PROJECTG_API UGProjectAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UGProjectAttributeSet();

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	UPROPERTY(BlueprintReadOnly, Category = "Vital")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UGProjectAttributeSet, Health);

	UPROPERTY(BlueprintReadOnly, Category = "Vital")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UGProjectAttributeSet, MaxHealth);

	UPROPERTY(BlueprintReadOnly, Category = "Vital")
	FGameplayAttributeData SP;
	ATTRIBUTE_ACCESSORS(UGProjectAttributeSet, SP);

	UPROPERTY(BlueprintReadOnly, Category = "Vital")
	FGameplayAttributeData MaxSP;
	ATTRIBUTE_ACCESSORS(UGProjectAttributeSet, MaxSP);

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FGameplayAttributeData MoveSpeed;
	ATTRIBUTE_ACCESSORS(UGProjectAttributeSet, MoveSpeed);

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FGameplayAttributeData AttackPower;
	ATTRIBUTE_ACCESSORS(UGProjectAttributeSet, AttackPower);

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FGameplayAttributeData Defense;
	ATTRIBUTE_ACCESSORS(UGProjectAttributeSet, Defense);

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FGameplayAttributeData KnockbackPower;
	ATTRIBUTE_ACCESSORS(UGProjectAttributeSet, KnockbackPower);

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FGameplayAttributeData KnockbackResistance;
	ATTRIBUTE_ACCESSORS(UGProjectAttributeSet, KnockbackResistance);

private:
	void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;
};
