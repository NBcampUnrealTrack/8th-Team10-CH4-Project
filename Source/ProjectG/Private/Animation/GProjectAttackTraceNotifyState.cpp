// Copyright Epic Games, Inc. All Rights Reserved.

#include "Animation/GProjectAttackTraceNotifyState.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Components/SkeletalMeshComponent.h"
#include "GProjectGameplayTags.h"

void UGProjectAttackTraceNotifyState::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	SendTraceEvent(MeshComp, GProjectGameplayTags::Event_Combat_Attack_Trace_Begin);
}

void UGProjectAttackTraceNotifyState::NotifyTick(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float FrameDeltaTime,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	SendTraceEvent(MeshComp, GProjectGameplayTags::Event_Combat_Attack_Trace_Tick);
}

void UGProjectAttackTraceNotifyState::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	SendTraceEvent(MeshComp, GProjectGameplayTags::Event_Combat_Attack_Trace_End);
}

void UGProjectAttackTraceNotifyState::SendTraceEvent(
	USkeletalMeshComponent* MeshComp,
	const FGameplayTag& EventTag) const
{
	AActor* OwnerActor = MeshComp ? MeshComp->GetOwner() : nullptr;
	if (!OwnerActor || !OwnerActor->HasAuthority())
	{
		return;
	}

	FGameplayEventData EventData;
	EventData.EventTag = EventTag;
	EventData.Instigator = OwnerActor;
	EventData.Target = OwnerActor;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OwnerActor, EventTag, EventData);
}
