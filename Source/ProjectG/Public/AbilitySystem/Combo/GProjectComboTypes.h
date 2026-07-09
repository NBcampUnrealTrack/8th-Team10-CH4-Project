// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/GProjectAbilitySystemLibrary.h"
#include "GProjectComboTypes.generated.h"

UENUM(BlueprintType)
enum class EGProjectAttackInput : uint8
{
	Basic,
	Strong
};

USTRUCT(BlueprintType)
struct FGProjectComboStep
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
	TArray<EGProjectAttackInput> InputSequence;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
	FName MontageSection;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
	FGProjectDamageEffectParams DamageParams;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cost", meta = (ClampMin = "0.0"))
	float StepSPCost = 0.0f;
};
