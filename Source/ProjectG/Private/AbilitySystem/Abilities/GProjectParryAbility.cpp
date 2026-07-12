// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Abilities/GProjectParryAbility.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "GProjectGameplayTags.h"

UGProjectParryAbility::UGProjectParryAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	bRetriggerInstancedAbility = true;

	StartupInputTag = GProjectGameplayTags::InputTag_Combat_Parry;

	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(GProjectGameplayTags::Ability_Combat_Parry);
	SetAssetTags(AssetTags);

	ActivationOwnedTags.AddTag(GProjectGameplayTags::State_Combat_Parrying);
	ActivationBlockedTags.AddTag(GProjectGameplayTags::State_Character_Dead);
	ActivationBlockedTags.AddTag(GProjectGameplayTags::State_Combat_Hitstun);
	ActivationBlockedTags.AddTag(GProjectGameplayTags::State_Combat_Knockdown);
	ActivationBlockedTags.AddTag(GProjectGameplayTags::State_Combat_Attacking);
}

void UGProjectParryAbility::ActivateAbility(
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

	bFinishingParry = false;

	ParrySuccessTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		GProjectGameplayTags::Event_Combat_Parry_Success,
		nullptr,
		false,
		false);
	ParrySuccessTask->EventReceived.AddDynamic(this, &ThisClass::OnParrySuccess);
	ParrySuccessTask->ReadyForActivation();

	ParryWindowClosedTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		GProjectGameplayTags::Event_Combat_Parry_Window_Close,
		nullptr,
		false,
		false);
	ParryWindowClosedTask->EventReceived.AddDynamic(this, &ThisClass::OnParryWindowClosed);
	ParryWindowClosedTask->ReadyForActivation();

	if (ParryMontage && MontagePlayRate > 0.0f)
	{
		MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			ParryMontage,
			MontagePlayRate,
			BlockStartSection);
		MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageFinished);
		MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageFinished);
		MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageFinished);
		MontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageFinished);
		MontageTask->ReadyForActivation();
		ConfigureParrySectionLinks();
	}
}

void UGProjectParryAbility::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	CloseParryWindow();
	MontageTask = nullptr;
	ParrySuccessTask = nullptr;
	ParryWindowClosedTask = nullptr;
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGProjectParryAbility::OnParryWindowClosed(FGameplayEventData Payload)
{
	if (bFinishingParry)
	{
		return;
	}

	if (ParryMontage && !EndSection.IsNone())
	{
		bFinishingParry = true;
		if (JumpToParrySection(EndSection))
		{
			return;
		}
	}

	FinishParry(false);
}

void UGProjectParryAbility::OnParrySuccess(FGameplayEventData Payload)
{
	if (bFinishingParry)
	{
		return;
	}

	if (ParryMontage && !SuccessSection.IsNone())
	{
		bFinishingParry = true;
		if (JumpToParrySection(SuccessSection))
		{
			return;
		}
	}

	FinishParry(false);
}

void UGProjectParryAbility::OnMontageFinished()
{
	FinishParry(false);
}

void UGProjectParryAbility::CloseParryWindow() const
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		if (ASC->HasMatchingGameplayTag(GProjectGameplayTags::State_Combat_ParryWindow))
		{
			ASC->RemoveLooseGameplayTag(GProjectGameplayTags::State_Combat_ParryWindow);
		}
	}
}

void UGProjectParryAbility::ConfigureParrySectionLinks() const
{
	if (!ParryMontage || BlockStartSection.IsNone() || LoopSection.IsNone())
	{
		return;
	}

	if (const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo())
	{
		if (UAnimInstance* AnimInstance = ActorInfo->GetAnimInstance())
		{
			AnimInstance->Montage_SetNextSection(BlockStartSection, LoopSection, ParryMontage);
		}
	}
}

bool UGProjectParryAbility::JumpToParrySection(FName SectionName) const
{
	if (!ParryMontage || SectionName.IsNone())
	{
		return false;
	}

	if (const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo())
	{
		if (UAnimInstance* AnimInstance = ActorInfo->GetAnimInstance())
		{
			if (!AnimInstance->Montage_IsPlaying(ParryMontage))
			{
				return false;
			}

			AnimInstance->Montage_JumpToSection(SectionName, ParryMontage);
			return true;
		}
	}

	return false;
}

void UGProjectParryAbility::FinishParry(bool bWasCancelled)
{
	if (IsActive())
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, bWasCancelled);
	}
}
