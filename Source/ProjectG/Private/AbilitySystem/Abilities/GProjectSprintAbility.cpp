// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Abilities/GProjectSprintAbility.h"

#include "Abilities/Tasks/AbilityTask_WaitAttributeChangeThreshold.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/GProjectAttributeSet.h"
#include "Character/GProjectCharacter.h"
#include "GameplayEffect.h"
#include "GProjectGameplayTags.h"

UGProjectSprintAbility::UGProjectSprintAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(GProjectGameplayTags::Ability_Movement_Sprint);
	SetAssetTags(AssetTags);

	ActivationBlockedTags.AddTag(GProjectGameplayTags::State_Character_Dead);
	ActivationBlockedTags.AddTag(GProjectGameplayTags::State_Combat_Hitstun);
	ActivationBlockedTags.AddTag(GProjectGameplayTags::State_Combat_Knockdown);
}

void UGProjectSprintAbility::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AGProjectCharacter* Character = Cast<AGProjectCharacter>(GetAvatarActorFromActorInfo());
	if (!Character)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	Character->StartSprint();

	if (ActorInfo->IsNetAuthority() && SprintDrainGameplayEffectClass)
	{
		SprintDrainEffectHandle = ApplyGameplayEffectToOwner(
			Handle,
			ActorInfo,
			ActivationInfo,
			SprintDrainGameplayEffectClass->GetDefaultObject<UGameplayEffect>(),
			1.0f);
	}

	SPThresholdTask = UAbilityTask_WaitAttributeChangeThreshold::WaitForAttributeChangeThreshold(
		this,
		UGProjectAttributeSet::GetSPAttribute(),
		EWaitAttributeChangeComparison::LessThanOrEqualTo,
		0.0f,
		true);
	SPThresholdTask->OnChange.AddDynamic(this, &ThisClass::OnSPThresholdReached);
	SPThresholdTask->ReadyForActivation();
}

void UGProjectSprintAbility::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	if (AGProjectCharacter* Character = Cast<AGProjectCharacter>(GetAvatarActorFromActorInfo()))
	{
		Character->StopSprint();
	}

	if (ActorInfo && ActorInfo->IsNetAuthority() && SprintDrainEffectHandle.IsValid())
	{
		ActorInfo->AbilitySystemComponent->RemoveActiveGameplayEffect(SprintDrainEffectHandle);
	}

	SprintDrainEffectHandle.Invalidate();
	SPThresholdTask = nullptr;
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGProjectSprintAbility::InputReleased(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	FinishSprint();
}

bool UGProjectSprintAbility::CheckCost(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags))
	{
		return false;
	}

	const UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	return ASC && ASC->GetNumericAttribute(UGProjectAttributeSet::GetSPAttribute()) > 0.0f;
}

void UGProjectSprintAbility::OnSPThresholdReached(bool bMatchesComparison, float CurrentValue)
{
	if (bMatchesComparison)
	{
		FinishSprint();
	}
}

void UGProjectSprintAbility::FinishSprint()
{
	if (IsActive())
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}
