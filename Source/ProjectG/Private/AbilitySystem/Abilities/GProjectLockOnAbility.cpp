// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Abilities/GProjectLockOnAbility.h"

#include "Abilities/Tasks/AbilityTask_WaitAttributeChangeThreshold.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/GProjectAttributeSet.h"
#include "Character/GProjectCharacter.h"
#include "GameplayEffect.h"
#include "GProjectGameplayTags.h"
#include "Targeting/GProjectLockOnComponent.h"

UGProjectLockOnAbility::UGProjectLockOnAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	StartupInputTag = GProjectGameplayTags::InputTag_Targeting_LockOn;

	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(GProjectGameplayTags::Ability_Targeting_LockOn);
	SetAssetTags(AssetTags);

	ActivationOwnedTags.AddTag(GProjectGameplayTags::State_Targeting_LockOn);

	ActivationBlockedTags.AddTag(GProjectGameplayTags::State_Character_Dead);
	ActivationBlockedTags.AddTag(GProjectGameplayTags::State_Combat_Hitstun);
	ActivationBlockedTags.AddTag(GProjectGameplayTags::State_Combat_Knockdown);
}

void UGProjectLockOnAbility::ActivateAbility(
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
	UGProjectLockOnComponent* LockOnComponent = Character ? Character->GetLockOnComponent() : nullptr;
	if (!LockOnComponent || !LockOnComponent->StartLockOn())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	LockOnComponent->OnLockOnTargetChanged.AddDynamic(this, &ThisClass::OnLockOnTargetChanged);

	if (ActorInfo->IsNetAuthority() && LockOnDrainGameplayEffectClass)
	{
		LockOnDrainEffectHandle = ApplyGameplayEffectToOwner(
			Handle,
			ActorInfo,
			ActivationInfo,
			LockOnDrainGameplayEffectClass->GetDefaultObject<UGameplayEffect>(),
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

void UGProjectLockOnAbility::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	if (AGProjectCharacter* Character = Cast<AGProjectCharacter>(GetAvatarActorFromActorInfo()))
	{
		if (UGProjectLockOnComponent* LockOnComponent = Character->GetLockOnComponent())
		{
			LockOnComponent->OnLockOnTargetChanged.RemoveDynamic(this, &ThisClass::OnLockOnTargetChanged);
			LockOnComponent->ClearLockOn();
		}
	}

	if (ActorInfo && ActorInfo->IsNetAuthority() && LockOnDrainEffectHandle.IsValid())
	{
		ActorInfo->AbilitySystemComponent->RemoveActiveGameplayEffect(LockOnDrainEffectHandle);
	}

	LockOnDrainEffectHandle.Invalidate();
	SPThresholdTask = nullptr;
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGProjectLockOnAbility::InputReleased(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	FinishLockOn();
}

bool UGProjectLockOnAbility::CheckCost(
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

void UGProjectLockOnAbility::OnSPThresholdReached(bool bMatchesComparison, float CurrentValue)
{
	if (bMatchesComparison)
	{
		FinishLockOn();
	}
}

void UGProjectLockOnAbility::OnLockOnTargetChanged(AActor* NewTarget)
{
	if (!NewTarget)
	{
		FinishLockOn();
	}
}

void UGProjectLockOnAbility::FinishLockOn()
{
	if (IsActive())
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}
