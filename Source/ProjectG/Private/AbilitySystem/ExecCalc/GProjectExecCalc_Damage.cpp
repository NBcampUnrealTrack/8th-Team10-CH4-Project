// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/ExecCalc/GProjectExecCalc_Damage.h"

#include "AbilitySystem/GProjectAttributeSet.h"
#include "GProjectGameplayTags.h"

struct FGProjectDamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(AttackPower);
	DECLARE_ATTRIBUTE_CAPTUREDEF(Defense);

	FGProjectDamageStatics()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(UGProjectAttributeSet, AttackPower, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UGProjectAttributeSet, Defense, Target, false);
	}
};

static const FGProjectDamageStatics& DamageStatics()
{
	static FGProjectDamageStatics Statics;
	return Statics;
}

UGProjectExecCalc_Damage::UGProjectExecCalc_Damage()
{
	RelevantAttributesToCapture.Add(DamageStatics().AttackPowerDef);
	RelevantAttributesToCapture.Add(DamageStatics().DefenseDef);
}

void UGProjectExecCalc_Damage::Execute_Implementation(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvaluationParameters.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	float AttackPower = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
		DamageStatics().AttackPowerDef,
		EvaluationParameters,
		AttackPower);

	float Defense = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
		DamageStatics().DefenseDef,
		EvaluationParameters,
		Defense);

	const float BaseDamage = Spec.GetSetByCallerMagnitude(
		GProjectGameplayTags::Data_Combat_Damage,
		false,
		0.0f);

	if (BaseDamage <= 0.0f)
	{
		return;
	}

	const float RawDamage = BaseDamage + FMath::Max(AttackPower, 0.0f) - FMath::Max(Defense, 0.0f);
	const float FinalDamage = FMath::Max(RawDamage, 1.0f);

	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
		UGProjectAttributeSet::GetIncomingDamageAttribute(),
		EGameplayModOp::Additive,
		FinalDamage));
}
