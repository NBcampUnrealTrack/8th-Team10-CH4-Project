// Copyright Epic Games, Inc. All Rights Reserved.

#include "Animation/GProjectAttackHitNotify.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GProjectGameplayTags.h"

void UGProjectAttackHitNotify::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	AActor* OwnerActor = MeshComp ? MeshComp->GetOwner() : nullptr;
	if (!OwnerActor || !UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerActor))
	{
		return;
	}

	FGameplayEventData EventData;
	EventData.EventTag = GProjectGameplayTags::Event_Combat_Attack_Hit;
	EventData.Instigator = OwnerActor;
	EventData.Target = OwnerActor;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		OwnerActor,
		GProjectGameplayTags::Event_Combat_Attack_Hit,
		EventData
	);
}
