#include "Animation/GAnimEventNotify.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbilityTypes.h"

void UGAnimEventNotify::Notify(
    USkeletalMeshComponent* MeshComp,
    UAnimSequenceBase* Animation,
    const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    if (!MeshComp || !MeshComp->GetOwner() || !EventTag.IsValid())
    {
        return;
    }

    FGameplayEventData Payload;
    Payload.EventTag = EventTag;

    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
        MeshComp->GetOwner(),
        EventTag,
        Payload);
}