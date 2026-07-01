// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Abilities/GProjectKnockdownAbility.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "GProjectGameplayTags.h"

UGProjectKnockdownAbility::UGProjectKnockdownAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
	bRetriggerInstancedAbility = true;

	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(GProjectGameplayTags::Ability_Combat_Knockdown);
	SetAssetTags(AssetTags);

	ActivationOwnedTags.AddTag(GProjectGameplayTags::State_Combat_Knockdown);
	ActivationOwnedTags.AddTag(GProjectGameplayTags::State_Character_Invulnerable);
	ActivationBlockedTags.AddTag(GProjectGameplayTags::State_Character_Dead);

	FAbilityTriggerData Trigger;
	Trigger.TriggerTag = GProjectGameplayTags::Event_Combat_Knockdown;
	Trigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(Trigger);
}

void UGProjectKnockdownAbility::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	const int32 FallSectionIndex = KnockdownMontage ? KnockdownMontage->GetSectionIndex(FallSection) : INDEX_NONE;
	if (!KnockdownMontage || FallSectionIndex == INDEX_NONE || MontagePlayRate <= 0.0f)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const float GroundTime = TriggerEventData ? FMath::Max(TriggerEventData->EventMagnitude, 0.0f) : 0.0f;
	const float FallTime = KnockdownMontage->GetSectionLength(FallSectionIndex) / MontagePlayRate;

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		KnockdownMontage,
		MontagePlayRate,
		FallSection);
	MontageTask->OnCompleted.AddDynamic(this, &ThisClass::FinishKnockdown);
	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::FinishKnockdown);
	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::FinishKnockdown);
	MontageTask->ReadyForActivation();

	GetUpDelayTask = UAbilityTask_WaitDelay::WaitDelay(this, FallTime + GroundTime);
	GetUpDelayTask->OnFinish.AddDynamic(this, &ThisClass::BeginGetUp);
	GetUpDelayTask->ReadyForActivation();
}

void UGProjectKnockdownAbility::BeginGetUp()
{
	UAnimInstance* AnimInstance = CurrentActorInfo ? CurrentActorInfo->GetAnimInstance() : nullptr;
	if (!AnimInstance || !KnockdownMontage)
	{
		FinishKnockdown();
		return;
	}

	AnimInstance->Montage_SetNextSection(DownLoopSection, GetUpSection, KnockdownMontage);
}

void UGProjectKnockdownAbility::FinishKnockdown()
{
	if (IsActive())
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}
