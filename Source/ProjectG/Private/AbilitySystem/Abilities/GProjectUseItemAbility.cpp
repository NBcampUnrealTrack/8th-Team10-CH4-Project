#include "AbilitySystem/Abilities/GProjectUseItemAbility.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Character/GProjectCharacter.h"
#include "GameFramework/Character.h"
#include "GProjectGameplayTags.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Item/GProjectItemActorBase.h"
#include "Item/GProjectItemHolderComponent.h"

UGProjectUseItemAbility::UGProjectUseItemAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
    StartupInputTag = GProjectGameplayTags::InputTag_Interaction_Interact;

    ActivationBlockedTags.AddTag(GProjectGameplayTags::State_Character_Dead);
    ActivationBlockedTags.AddTag(GProjectGameplayTags::State_Combat_Hitstun);
    ActivationBlockedTags.AddTag(GProjectGameplayTags::State_Combat_Knockdown);
    ActivationBlockedTags.AddTag(GProjectGameplayTags::State_Movement_Airborne);
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

    bUsed = false;

    AGProjectCharacter* Character = Cast<AGProjectCharacter>(GetAvatarActorFromActorInfo());
    UGProjectItemHolderComponent* Holder = Character ? Character->GetItemHolderComponent() : nullptr;
    AGProjectItemActorBase* HeldItem = Holder ? Holder->GetHeldItem() : nullptr;
    if (!Character || !Holder || !HeldItem || !HeldItem->CanUse(Character))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    if (ACharacter* Char = Cast<ACharacter>(Character))
    {
        if (UCharacterMovementComponent* Move = Char->GetCharacterMovement())
        {
            Move->DisableMovement();
        }
    }

    UAnimMontage* ItemUseMontage = HeldItem->GetUseMontage();

    if (!ItemUseMontage)
    {
        DoUse();
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    EventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
        this,
        GProjectGameplayTags::Event_Interaction_UseItem,
        nullptr,
        false,
        false);
    EventTask->EventReceived.AddDynamic(this, &ThisClass::OnUseEvent);
    EventTask->ReadyForActivation();

    MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
        this,
        NAME_None,
        ItemUseMontage,
        2.5f);
    MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageFinished);
    MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageFinished);
    MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageFinished);
    MontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageFinished);
    MontageTask->ReadyForActivation();
}

void UGProjectUseItemAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    if (ACharacter* Char = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
    {
        if (UCharacterMovementComponent* Move = Char->GetCharacterMovement())
        {
            Move->SetMovementMode(MOVE_Walking);
        }
    }

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGProjectUseItemAbility::OnUseEvent(FGameplayEventData Payload)
{
    DoUse();
}

void UGProjectUseItemAbility::OnMontageFinished()
{
    if (!bUsed)
    {
        DoUse();
    }

    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGProjectUseItemAbility::DoUse()
{
    if (bUsed)
    {
        return;
    }
    bUsed = true;

    if (AGProjectCharacter* Character = Cast<AGProjectCharacter>(GetAvatarActorFromActorInfo()))
    {
        if (UGProjectItemHolderComponent* Holder = Character->GetItemHolderComponent())
        {
            Holder->UseHeldItem();
        }
    }
}
