#include "AbilitySystem/Abilities/GProjectUseItemAbility.h"
#include "Item/GItemHolderComponent.h"
#include "GProjectGameplayTags.h"

UGProjectUseItemAbility::UGProjectUseItemAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

    ActivationBlockedTags.AddTag(GProjectGameplayTags::State_Character_Dead);
    ActivationBlockedTags.AddTag(GProjectGameplayTags::State_Combat_Hitstun);
    ActivationBlockedTags.AddTag(GProjectGameplayTags::State_Combat_Knockdown);
}

void UGProjectUseItemAbility::ActivateAbility(
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
            Holder->UseHeldItem();
        }
    }

    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}