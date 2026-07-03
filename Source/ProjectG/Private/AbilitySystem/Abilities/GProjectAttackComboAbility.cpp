// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Abilities/GProjectAttackComboAbility.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Combo/GProjectComboData.h"
#include "Animation/AnimInstance.h"
#include "Animation/GProjectAttackTraceNotifyState.h"
#include "Character/GProjectCharacter.h"
#include "Components/MeshComponent.h"
#include "DrawDebugHelpers.h"
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
	ActivationBlockedTags.AddTag(GProjectGameplayTags::State_Combat_Hitstun);
	ActivationBlockedTags.AddTag(GProjectGameplayTags::State_Combat_Knockdown);

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
	const AGProjectCharacter* Character = Cast<AGProjectCharacter>(GetAvatarActorFromActorInfo());
	ComboData = Character ? Character->GetActiveComboData() : nullptr;
	if (!ComboData)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

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
	bHasPreviousUnarmedTraceLocation = false;
	PreviousUnarmedTraceLocation = FVector::ZeroVector;
	CurrentUnarmedTraceSocket = NAME_None;
	EventTask = nullptr;
	MontageTask = nullptr;
	ComboData = nullptr;

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
	else if (EventTag.MatchesTagExact(GProjectGameplayTags::Event_Combat_Attack_Trace_Begin))
	{
		SyncCurrentStepFromMontage();
		const UGProjectAttackTraceNotifyState* TraceNotify =
			Cast<UGProjectAttackTraceNotifyState>(Payload.OptionalObject);
		BeginCurrentStepTrace(TraceNotify ? TraceNotify->GetTraceSocketName() : NAME_None);
	}
	else if (EventTag.MatchesTagExact(GProjectGameplayTags::Event_Combat_Attack_Trace_Tick))
	{
		SyncCurrentStepFromMontage();
		ApplyCurrentStepHit();
	}
	else if (EventTag.MatchesTagExact(GProjectGameplayTags::Event_Combat_Attack_Trace_End))
	{
		EndCurrentStepTrace();
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
	if (!ComboData)
	{
		FinishAbility();
		return;
	}

	TArray<EGProjectAttackInput> InitialSequence{ AttackInput };
	CurrentComboStepIndex = FindComboStepIndex(InitialSequence);
	if (!ComboData->ComboSteps.IsValidIndex(CurrentComboStepIndex) || !ComboData->ComboMontage)
	{
		FinishAbility();
		return;
	}

	HitActorsThisStep.Reset();
	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		ComboData->ComboMontage,
		1.0f,
		ComboData->ComboSteps[CurrentComboStepIndex].MontageSection
	);
	MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageEnded);
	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageEnded);
	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageEnded);
	MontageTask->ReadyForActivation();
}

void UGProjectAttackComboAbility::AddBufferedInput(EGProjectAttackInput AttackInput)
{
	UWorld* World = GetWorld();
	if (!World || !ComboData)
	{
		return;
	}

	RemoveExpiredInputs();
	const int32 BufferLimit = FMath::Max(ComboData->MaxBufferedInputs, 1);
	while (InputBuffer.Num() >= BufferLimit)
	{
		InputBuffer.RemoveAt(0);
	}

	FGProjectBufferedAttackInput& BufferedInput = InputBuffer.AddDefaulted_GetRef();
	BufferedInput.Input = AttackInput;
	BufferedInput.ExpireTime = World->GetTimeSeconds() + ComboData->InputBufferLifetime;
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
	if (!ComboData || !bComboWindowOpen || bNextSectionReserved ||
		!ComboData->ComboSteps.IsValidIndex(CurrentComboStepIndex) || !CurrentActorInfo)
	{
		return;
	}

	UAnimInstance* AnimInstance = CurrentActorInfo->GetAnimInstance();
	if (!AnimInstance || !ComboData->ComboMontage)
	{
		return;
	}

	RemoveExpiredInputs();
	while (!InputBuffer.IsEmpty())
	{
		const EGProjectAttackInput NextInput = InputBuffer[0].Input;
		InputBuffer.RemoveAt(0);

		TArray<EGProjectAttackInput> NextSequence = ComboData->ComboSteps[CurrentComboStepIndex].InputSequence;
		NextSequence.Add(NextInput);
		const int32 NextComboStepIndex = FindComboStepIndex(NextSequence);
		if (!ComboData->ComboSteps.IsValidIndex(NextComboStepIndex))
		{
			continue;
		}

		AnimInstance->Montage_SetNextSection(
			ComboData->ComboSteps[CurrentComboStepIndex].MontageSection,
			ComboData->ComboSteps[NextComboStepIndex].MontageSection,
			ComboData->ComboMontage
		);
		bNextSectionReserved = true;
		return;
	}
}

void UGProjectAttackComboAbility::SyncCurrentStepFromMontage()
{
	if (!ComboData || !CurrentActorInfo || !ComboData->ComboMontage)
	{
		return;
	}

	UAnimInstance* AnimInstance = CurrentActorInfo->GetAnimInstance();
	if (!AnimInstance)
	{
		return;
	}

	const int32 NewComboStepIndex = FindComboStepIndexBySection(
		AnimInstance->Montage_GetCurrentSection(ComboData->ComboMontage));
	if (ComboData->ComboSteps.IsValidIndex(NewComboStepIndex) && NewComboStepIndex != CurrentComboStepIndex)
	{
		CurrentComboStepIndex = NewComboStepIndex;
		HitActorsThisStep.Reset();
		EndCurrentStepTrace();
	}
}

void UGProjectAttackComboAbility::BeginCurrentStepTrace(FName TraceSocketName)
{
	bHasPreviousUnarmedTraceLocation = false;
	PreviousUnarmedTraceLocation = FVector::ZeroVector;
	CurrentUnarmedTraceSocket = TraceSocketName;

	AActor* Attacker = GetAvatarActorFromActorInfo();
	const FGProjectComboStep* CurrentComboStep = GetCurrentComboStep();
	const AGProjectCharacter* Character = Cast<AGProjectCharacter>(Attacker);
	UMeshComponent* TraceMesh = Character ? Character->GetMesh() : nullptr;
	if (!CurrentComboStep || ComboData->TraceType != EGProjectAttackTraceType::Unarmed ||
		!TraceMesh || CurrentUnarmedTraceSocket.IsNone())
	{
		return;
	}

	PreviousUnarmedTraceLocation = TraceMesh->GetSocketLocation(CurrentUnarmedTraceSocket);
	bHasPreviousUnarmedTraceLocation = true;
}

void UGProjectAttackComboAbility::EndCurrentStepTrace()
{
	bHasPreviousUnarmedTraceLocation = false;
	PreviousUnarmedTraceLocation = FVector::ZeroVector;
	CurrentUnarmedTraceSocket = NAME_None;
}

void UGProjectAttackComboAbility::ApplyCurrentStepHit()
{
	AActor* Attacker = GetAvatarActorFromActorInfo();
	const FGProjectComboStep* CurrentComboStep = GetCurrentComboStep();
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	if (!ComboData || !Attacker || !Attacker->HasAuthority() || !CurrentComboStep || !SourceASC)
	{
		return;
	}

	const AGProjectCharacter* Character = Cast<AGProjectCharacter>(Attacker);
	UMeshComponent* TraceMesh = nullptr;
	if (Character)
	{
		TraceMesh = ComboData->TraceType == EGProjectAttackTraceType::Unarmed
			? Character->GetMesh()
			: Character->GetAttackTraceMesh();
	}
	if (!TraceMesh || !GetWorld())
	{
		return;
	}

	FVector TraceStart;
	FVector TraceEnd;
	if (ComboData->TraceType == EGProjectAttackTraceType::Unarmed)
	{
		if (CurrentUnarmedTraceSocket.IsNone())
		{
			return;
		}

		TraceEnd = TraceMesh->GetSocketLocation(CurrentUnarmedTraceSocket);
		TraceStart = bHasPreviousUnarmedTraceLocation ? PreviousUnarmedTraceLocation : TraceEnd;
		PreviousUnarmedTraceLocation = TraceEnd;
		bHasPreviousUnarmedTraceLocation = true;
	}
	else
	{
		if (Character->GetAttackTraceStartSocketName().IsNone() ||
			Character->GetAttackTraceEndSocketName().IsNone())
		{
			return;
		}

		TraceStart = TraceMesh->GetSocketLocation(Character->GetAttackTraceStartSocketName());
		TraceEnd = TraceMesh->GetSocketLocation(Character->GetAttackTraceEndSocketName());
	}

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(AttackSocketTrace), false, Attacker);
	TArray<FHitResult> HitResults;
	GetWorld()->SweepMultiByChannel(
		HitResults,
		TraceStart,
		TraceEnd,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(ComboData->TraceRadius),
		QueryParams);

#if ENABLE_DRAW_DEBUG
	if (ComboData->bDrawDebugTrace)
	{
		const FVector TraceDirection = TraceEnd - TraceStart;
		const FVector TraceCenter = (TraceStart + TraceEnd) * 0.5f;
		const FQuat TraceRotation = TraceDirection.IsNearlyZero()
			? FQuat::Identity
			: FRotationMatrix::MakeFromZ(TraceDirection).ToQuat();
		const float CapsuleHalfHeight = TraceDirection.Size() * 0.5f + ComboData->TraceRadius;

		DrawDebugCapsule(
			GetWorld(),
			TraceCenter,
			CapsuleHalfHeight,
			ComboData->TraceRadius,
			TraceRotation,
			HitResults.IsEmpty() ? FColor::Green : FColor::Red,
			false,
			ComboData->DebugTraceDuration,
			0,
			1.5f);
	}
#endif

	for (const FHitResult& HitResult : HitResults)
	{
		AActor* Target = HitResult.GetActor();
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

		const FGameplayEffectContextHandle DamageContext = UGProjectAbilitySystemLibrary::ApplyDamageEffect(
			DamageParams,
			DamageGameplayEffectClass);
		if (!DamageContext.IsValid())
		{
			continue;
		}

		if (DamageParams.CausesKnockdown())
		{
			UGProjectAbilitySystemLibrary::SendKnockdownEvent(DamageParams);
		}
		else
		{
			UGProjectAbilitySystemLibrary::ApplyHitstunEffect(DamageParams, HitstunGameplayEffectClass);
			UGProjectAbilitySystemLibrary::SendHitReactEvent(DamageParams);
		}
		HitActorsThisStep.Add(TargetPtr);

		if (ACharacter* TargetCharacter = Cast<ACharacter>(Target); !DamageParams.KnockbackForce.IsNearlyZero())
		{
			FVector LaunchVelocity = DamageParams.KnockbackForce;
			LaunchVelocity.Z = 200.0f;
			TargetCharacter->LaunchCharacter(LaunchVelocity, true, true);
		}
	}
}

int32 UGProjectAttackComboAbility::FindComboStepIndex(const TArray<EGProjectAttackInput>& InputSequence) const
{
	if (!ComboData)
	{
		return INDEX_NONE;
	}

	return ComboData->ComboSteps.IndexOfByPredicate(
		[&InputSequence](const FGProjectComboStep& ComboStep)
		{
			return ComboStep.InputSequence == InputSequence;
		}
	);
}

int32 UGProjectAttackComboAbility::FindComboStepIndexBySection(FName MontageSection) const
{
	if (!ComboData)
	{
		return INDEX_NONE;
	}

	return ComboData->ComboSteps.IndexOfByPredicate(
		[MontageSection](const FGProjectComboStep& ComboStep)
		{
			return ComboStep.MontageSection == MontageSection;
		}
	);
}

const FGProjectComboStep* UGProjectAttackComboAbility::GetCurrentComboStep() const
{
	return ComboData && ComboData->ComboSteps.IsValidIndex(CurrentComboStepIndex)
		? &ComboData->ComboSteps[CurrentComboStepIndex]
		: nullptr;
}

void UGProjectAttackComboAbility::FinishAbility()
{
	if (IsActive())
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}
