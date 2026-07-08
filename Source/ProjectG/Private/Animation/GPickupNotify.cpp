// Copyright Epic Games, Inc. All Rights Reserved.

#include "Animation/GPickupNotify.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "GProjectGameplayTags.h"

void UGPickupNotify::Notify(
    USkeletalMeshComponent* MeshComp,
    UAnimSequenceBase* Animation,
    const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    if (!MeshComp || !MeshComp->GetOwner())
    {
        return;
    }

    FGameplayEventData Payload;
    Payload.EventTag = GProjectGameplayTags::Event_Interaction_Pickup;

    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
        MeshComp->GetOwner(),
        GProjectGameplayTags::Event_Interaction_Pickup,
        Payload);
}