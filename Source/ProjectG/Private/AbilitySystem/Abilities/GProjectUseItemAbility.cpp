#include "AbilitySystem/Abilities/GProjectUseItemAbility.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Item/GItemHolderComponent.h"
#include "GameFramework/Character.h"
#include "GProjectGameplayTags.h"
#include "GameFramework/CharacterMovementComponent.h"

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

    bUsed = false;

    AActor* Avatar = GetAvatarActorFromActorInfo();
    UGItemHolderComponent* Holder = Avatar ? Avatar->FindComponentByClass<UGItemHolderComponent>() : nullptr;
    if (!Holder || !Holder->HasHeldItem())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    if (ACharacter* Char = Cast<ACharacter>(Avatar))
    {
        if (UCharacterMovementComponent* Move = Char->GetCharacterMovement())
        {
            Move->DisableMovement();
        }
    }

    if (!UseMontage)
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
        UseMontage,
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

    if (AActor* Avatar = GetAvatarActorFromActorInfo())
    {
        if (UGItemHolderComponent* Holder = Avatar->FindComponentByClass<UGItemHolderComponent>())
        {
            Holder->UseHeldItem();
        }
    }
}