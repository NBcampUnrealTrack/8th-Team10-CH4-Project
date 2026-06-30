// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Abilities/GProjectHitReactAbility.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GProjectGameplayTags.h"

UGProjectHitReactAbility::UGProjectHitReactAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
	bRetriggerInstancedAbility = true;

	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(GProjectGameplayTags::Ability_Combat_HitReact);
	SetAssetTags(AssetTags);

	ActivationBlockedTags.AddTag(GProjectGameplayTags::State_Character_Dead);

	FAbilityTriggerData Trigger;
	Trigger.TriggerTag = GProjectGameplayTags::Event_Combat_HitReact;
	Trigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(Trigger);
}

void UGProjectHitReactAbility::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!HitReactMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		HitReactMontage);
	MontageTask->OnCompleted.AddDynamic(this, &ThisClass::FinishHitReact);
	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::FinishHitReact);
	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::FinishHitReact);
	MontageTask->ReadyForActivation();
}

void UGProjectHitReactAbility::FinishHitReact()
{
	if (IsActive())
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}
