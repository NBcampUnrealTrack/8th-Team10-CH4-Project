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

UENUM(BlueprintType)
enum class EGProjectAttackTraceType : uint8
{
	Unarmed,
	Weapon
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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit Detection")
	EGProjectAttackTraceType TraceType = EGProjectAttackTraceType::Unarmed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit Detection")
	FName UnarmedTraceSocket = TEXT("hand_r");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit Detection", meta = (ClampMin = "0.0"))
	float TraceRadius = 20.0f;
};
