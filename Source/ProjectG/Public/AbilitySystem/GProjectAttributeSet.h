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

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	// Meta attribute used to pass calculated damage into post-effect processing.
	UPROPERTY(BlueprintReadOnly, Category = "Meta")
	FGameplayAttributeData IncomingDamage;
	ATTRIBUTE_ACCESSORS(UGProjectAttributeSet, IncomingDamage);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Vital")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UGProjectAttributeSet, Health);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "Vital")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UGProjectAttributeSet, MaxHealth);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_SP, Category = "Vital")
	FGameplayAttributeData SP;
	ATTRIBUTE_ACCESSORS(UGProjectAttributeSet, SP);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxSP, Category = "Vital")
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

protected:
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth) const;

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const;

	UFUNCTION()
	void OnRep_SP(const FGameplayAttributeData& OldSP) const;

	UFUNCTION()
	void OnRep_MaxSP(const FGameplayAttributeData& OldMaxSP) const;

private:
	void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;
};
