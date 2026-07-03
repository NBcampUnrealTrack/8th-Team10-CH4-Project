#include "AbilitySystem/Abilities/GProjectPickupAbility.h"

#include "Item/GItemHolderComponent.h"
#include "GameFramework/Character.h"
#include "GProjectGameplayTags.h"

UGProjectPickupAbility::UGProjectPickupAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    ActivationBlockedTags.AddTag(GProjectGameplayTags::State_Character_Dead);
    ActivationBlockedTags.AddTag(GProjectGameplayTags::State_Combat_Hitstun);
    ActivationBlockedTags.AddTag(GProjectGameplayTags::State_Combat_Knockdown);
}

void UGProjectPickupAbility::ActivateAbility(
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

    if (AActor* Avatar = GetAvatarActorFromActorInfo())
    {
        if (UGItemHolderComponent* Holder = Avatar->FindComponentByClass<UGItemHolderComponent>())
        {
            Holder->TryPickupNearby();
        }
    }

    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}