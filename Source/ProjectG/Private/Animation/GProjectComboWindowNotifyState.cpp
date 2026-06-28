// Copyright Epic Games, Inc. All Rights Reserved.

#include "Animation/GProjectComboWindowNotifyState.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GProjectGameplayTags.h"

namespace
{
	void SendComboEvent(USkeletalMeshComponent* MeshComp, const FGameplayTag& EventTag)
	{
		AActor* OwnerActor = MeshComp ? MeshComp->GetOwner() : nullptr;
		if (!OwnerActor || !UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerActor))
		{
			return;
		}

		FGameplayEventData EventData;
		EventData.EventTag = EventTag;
		EventData.Instigator = OwnerActor;
		EventData.Target = OwnerActor;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OwnerActor, EventTag, EventData);
	}
}

void UGProjectComboWindowNotifyState::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	SendComboEvent(MeshComp, GProjectGameplayTags::Event_Combat_Combo_Window_Open);
}

void UGProjectComboWindowNotifyState::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	SendComboEvent(MeshComp, GProjectGameplayTags::Event_Combat_Combo_Window_Close);
}
