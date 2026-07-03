// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Abilities/GProjectHitReactAbility.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GameFramework/Actor.h"
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
	ActivationBlockedTags.AddTag(GProjectGameplayTags::State_Combat_Knockdown);

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
		HitReactMontage,
		1.0f,
		DetermineHitSection(TriggerEventData));
	MontageTask->OnCompleted.AddDynamic(this, &ThisClass::FinishHitReact);
	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::FinishHitReact);
	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::FinishHitReact);
	MontageTask->ReadyForActivation();
}

FName UGProjectHitReactAbility::DetermineHitSection(const FGameplayEventData* TriggerEventData) const
{
	const AActor* TargetActor = GetAvatarActorFromActorInfo();
	const AActor* Attacker = TriggerEventData ? TriggerEventData->Instigator.Get() : nullptr;
	if (!TargetActor || !Attacker)
	{
		return FrontSection;
	}

	FVector ToAttacker = Attacker->GetActorLocation() - TargetActor->GetActorLocation();
	ToAttacker.Z = 0.0f;
	if (!ToAttacker.Normalize())
	{
		return FrontSection;
	}

	FVector Forward = TargetActor->GetActorForwardVector();
	Forward.Z = 0.0f;
	Forward.Normalize();

	FVector Right = TargetActor->GetActorRightVector();
	Right.Z = 0.0f;
	Right.Normalize();

	const float ForwardDot = FVector::DotProduct(Forward, ToAttacker);
	const float RightDot = FVector::DotProduct(Right, ToAttacker);
	if (FMath::Abs(ForwardDot) >= FMath::Abs(RightDot))
	{
		return ForwardDot >= 0.0f ? FrontSection : BackSection;
	}

	return RightDot >= 0.0f ? RightSection : LeftSection;
}

void UGProjectHitReactAbility::FinishHitReact()
{
	if (IsActive())
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}
