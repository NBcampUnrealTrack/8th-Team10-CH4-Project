// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Abilities/GProjectAttackComboAbility.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GProjectGameplayTags.h"

UGProjectAttackComboAbility::UGProjectAttackComboAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(GProjectGameplayTags::Ability_Combat_Attack);
	SetAssetTags(AssetTags);

	ActivationOwnedTags.AddTag(GProjectGameplayTags::State_Combat_Attacking);
	ActivationBlockedTags.AddTag(GProjectGameplayTags::State_Character_Dead);

	FAbilityTriggerData BasicAttackTrigger;
	BasicAttackTrigger.TriggerTag = GProjectGameplayTags::Event_Input_Combat_BasicAttack;
	BasicAttackTrigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(BasicAttackTrigger);

	FAbilityTriggerData StrongAttackTrigger;
	StrongAttackTrigger.TriggerTag = GProjectGameplayTags::Event_Input_Combat_StrongAttack;
	StrongAttackTrigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(StrongAttackTrigger);
}

void UGProjectAttackComboAbility::ActivateAbility(
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

	EventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		GProjectGameplayTags::Event_Root,
		nullptr,
		false,
		false
	);
	EventTask->EventReceived.AddDynamic(this, &ThisClass::OnGameplayEvent);
	EventTask->ReadyForActivation();
}

void UGProjectAttackComboAbility::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	InputBuffer.Reset();
	HitActorsThisStep.Reset();
	CurrentComboStepIndex = INDEX_NONE;
	bComboWindowOpen = false;
	bNextSectionReserved = false;
	EventTask = nullptr;
	MontageTask = nullptr;

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGProjectAttackComboAbility::OnGameplayEvent(FGameplayEventData Payload)
{
	const FGameplayTag EventTag = Payload.EventTag;

	if (EventTag.MatchesTagExact(GProjectGameplayTags::Event_Input_Combat_BasicAttack))
	{
		HandleAttackInput(EGProjectAttackInput::Basic);
	}
	else if (EventTag.MatchesTagExact(GProjectGameplayTags::Event_Input_Combat_StrongAttack))
	{
		HandleAttackInput(EGProjectAttackInput::Strong);
	}
	else if (EventTag.MatchesTagExact(GProjectGameplayTags::Event_Combat_Combo_Window_Open))
	{
		SyncCurrentStepFromMontage();
		bComboWindowOpen = true;
		bNextSectionReserved = false;
		TryReserveNextSection();
	}
	else if (EventTag.MatchesTagExact(GProjectGameplayTags::Event_Combat_Combo_Window_Close))
	{
		bComboWindowOpen = false;
	}
	else if (EventTag.MatchesTagExact(GProjectGameplayTags::Event_Combat_Attack_Hit))
	{
		SyncCurrentStepFromMontage();
		ApplyCurrentStepHit();
	}
}

void UGProjectAttackComboAbility::OnMontageEnded()
{
	FinishAbility();
}

void UGProjectAttackComboAbility::HandleAttackInput(EGProjectAttackInput AttackInput)
{
	if (CurrentComboStepIndex == INDEX_NONE)
	{
		StartCombo(AttackInput);
		return;
	}

	AddBufferedInput(AttackInput);
	if (bComboWindowOpen && !bNextSectionReserved)
	{
		TryReserveNextSection();
	}
}

void UGProjectAttackComboAbility::StartCombo(EGProjectAttackInput AttackInput)
{
	TArray<EGProjectAttackInput> InitialSequence{ AttackInput };
	CurrentComboStepIndex = FindComboStepIndex(InitialSequence);
	if (!ComboSteps.IsValidIndex(CurrentComboStepIndex) || !ComboMontage)
	{
		FinishAbility();
		return;
	}

	HitActorsThisStep.Reset();
	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		ComboMontage,
		1.0f,
		ComboSteps[CurrentComboStepIndex].MontageSection
	);
	MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageEnded);
	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageEnded);
	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageEnded);
	MontageTask->ReadyForActivation();
}

void UGProjectAttackComboAbility::AddBufferedInput(EGProjectAttackInput AttackInput)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	RemoveExpiredInputs();
	const int32 BufferLimit = FMath::Max(MaxBufferedInputs, 1);
	while (InputBuffer.Num() >= BufferLimit)
	{
		InputBuffer.RemoveAt(0);
	}

	FGProjectBufferedAttackInput& BufferedInput = InputBuffer.AddDefaulted_GetRef();
	BufferedInput.Input = AttackInput;
	BufferedInput.ExpireTime = World->GetTimeSeconds() + InputBufferLifetime;
}

void UGProjectAttackComboAbility::RemoveExpiredInputs()
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		InputBuffer.Reset();
		return;
	}

	const double CurrentTime = World->GetTimeSeconds();
	InputBuffer.RemoveAll(
		[CurrentTime](const FGProjectBufferedAttackInput& BufferedInput)
		{
			return BufferedInput.ExpireTime < CurrentTime;
		}
	);
}

void UGProjectAttackComboAbility::TryReserveNextSection()
{
	if (!bComboWindowOpen || bNextSectionReserved || !ComboSteps.IsValidIndex(CurrentComboStepIndex) || !CurrentActorInfo)
	{
		return;
	}

	UAnimInstance* AnimInstance = CurrentActorInfo->GetAnimInstance();
	if (!AnimInstance || !ComboMontage)
	{
		return;
	}

	RemoveExpiredInputs();
	while (!InputBuffer.IsEmpty())
	{
		const EGProjectAttackInput NextInput = InputBuffer[0].Input;
		InputBuffer.RemoveAt(0);

		TArray<EGProjectAttackInput> NextSequence = ComboSteps[CurrentComboStepIndex].InputSequence;
		NextSequence.Add(NextInput);
		const int32 NextComboStepIndex = FindComboStepIndex(NextSequence);
		if (!ComboSteps.IsValidIndex(NextComboStepIndex))
		{
			continue;
		}

		AnimInstance->Montage_SetNextSection(
			ComboSteps[CurrentComboStepIndex].MontageSection,
			ComboSteps[NextComboStepIndex].MontageSection,
			ComboMontage
		);
		bNextSectionReserved = true;
		return;
	}
}

void UGProjectAttackComboAbility::SyncCurrentStepFromMontage()
{
	if (!CurrentActorInfo || !ComboMontage)
	{
		return;
	}

	UAnimInstance* AnimInstance = CurrentActorInfo->GetAnimInstance();
	if (!AnimInstance)
	{
		return;
	}

	const int32 NewComboStepIndex = FindComboStepIndexBySection(AnimInstance->Montage_GetCurrentSection(ComboMontage));
	if (ComboSteps.IsValidIndex(NewComboStepIndex) && NewComboStepIndex != CurrentComboStepIndex)
	{
		CurrentComboStepIndex = NewComboStepIndex;
		HitActorsThisStep.Reset();
	}
}

void UGProjectAttackComboAbility::ApplyCurrentStepHit()
{
	AActor* Attacker = GetAvatarActorFromActorInfo();
	const FGProjectComboStep* CurrentComboStep = GetCurrentComboStep();
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	if (!Attacker || !Attacker->HasAuthority() || !CurrentComboStep || !SourceASC)
	{
		return;
	}

	const FVector HitCenter = Attacker->GetActorLocation() + Attacker->GetActorForwardVector() * AttackRange;
	TArray<AActor*> Targets;
	TArray<AActor*> ActorsToIgnore{ Attacker };
	UGProjectAbilitySystemLibrary::GetLivePlayersWithinRadius(
		Attacker,
		Targets,
		ActorsToIgnore,
		AttackRadius,
		HitCenter
	);

	for (AActor* Target : Targets)
	{
		const TWeakObjectPtr<AActor> TargetPtr(Target);
		if (!Target || HitActorsThisStep.Contains(TargetPtr))
		{
			continue;
		}

		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
		if (!TargetASC)
		{
			continue;
		}

		FGProjectDamageEffectParams DamageParams = CurrentComboStep->DamageParams;
		DamageParams.SourceAbilitySystemComponent = SourceASC;
		DamageParams.TargetAbilitySystemComponent = TargetASC;
		UGProjectAbilitySystemLibrary::SetKnockbackDirection(
			DamageParams,
			Target->GetActorLocation() - Attacker->GetActorLocation()
		);

		UGProjectAbilitySystemLibrary::ApplyDamageEffect(DamageParams);
		HitActorsThisStep.Add(TargetPtr);

		if (ACharacter* TargetCharacter = Cast<ACharacter>(Target); !DamageParams.KnockbackForce.IsNearlyZero())
		{
			TargetCharacter->LaunchCharacter(DamageParams.KnockbackForce, true, true);
		}
	}
}

int32 UGProjectAttackComboAbility::FindComboStepIndex(const TArray<EGProjectAttackInput>& InputSequence) const
{
	return ComboSteps.IndexOfByPredicate(
		[&InputSequence](const FGProjectComboStep& ComboStep)
		{
			return ComboStep.InputSequence == InputSequence;
		}
	);
}

int32 UGProjectAttackComboAbility::FindComboStepIndexBySection(FName MontageSection) const
{
	return ComboSteps.IndexOfByPredicate(
		[MontageSection](const FGProjectComboStep& ComboStep)
		{
			return ComboStep.MontageSection == MontageSection;
		}
	);
}

const FGProjectComboStep* UGProjectAttackComboAbility::GetCurrentComboStep() const
{
	return ComboSteps.IsValidIndex(CurrentComboStepIndex) ? &ComboSteps[CurrentComboStepIndex] : nullptr;
}

void UGProjectAttackComboAbility::FinishAbility()
{
	if (IsActive())
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}
