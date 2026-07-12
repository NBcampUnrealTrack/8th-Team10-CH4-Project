// Copyright Epic Games, Inc. All Rights Reserved.

#include "Animation/GProjectParryWindowNotifyState.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GProjectGameplayTags.h"

namespace
{
	AActor* GetParryOwner(USkeletalMeshComponent* MeshComp)
	{
		return MeshComp ? MeshComp->GetOwner() : nullptr;
	}
}

void UGProjectParryWindowNotifyState::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	AActor* OwnerActor = GetParryOwner(MeshComp);
	UAbilitySystemComponent* ASC = OwnerActor ? UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerActor) : nullptr;
	if (!ASC)
	{
		return;
	}

	ASC->AddLooseGameplayTag(GProjectGameplayTags::State_Combat_ParryWindow);
}

void UGProjectParryWindowNotifyState::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	AActor* OwnerActor = GetParryOwner(MeshComp);
	UAbilitySystemComponent* ASC = OwnerActor ? UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerActor) : nullptr;
	if (!OwnerActor || !ASC)
	{
		return;
	}

	if (ASC->HasMatchingGameplayTag(GProjectGameplayTags::State_Combat_ParryWindow))
	{
		ASC->RemoveLooseGameplayTag(GProjectGameplayTags::State_Combat_ParryWindow);
	}

	FGameplayEventData EventData;
	EventData.EventTag = GProjectGameplayTags::Event_Combat_Parry_Window_Close;
	EventData.Instigator = OwnerActor;
	EventData.Target = OwnerActor;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		OwnerActor,
		GProjectGameplayTags::Event_Combat_Parry_Window_Close,
		EventData);
}
