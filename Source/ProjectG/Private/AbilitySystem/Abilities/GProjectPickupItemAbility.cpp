// Copyright Epic Games, Inc. All Rights Reserved.
#include "AbilitySystem/Abilities/GProjectPickupItemAbility.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Character/GProjectCharacter.h"
#include "Item/GProjectItemHolderComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GProjectGameplayTags.h"

UGProjectPickupItemAbility::UGProjectPickupItemAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
    ActivationBlockedTags.AddTag(GProjectGameplayTags::State_Character_Dead);
    ActivationBlockedTags.AddTag(GProjectGameplayTags::State_Combat_Hitstun);
    ActivationBlockedTags.AddTag(GProjectGameplayTags::State_Combat_Knockdown);
    ActivationBlockedTags.AddTag(GProjectGameplayTags::State_Character_Transformed);
    ActivationOwnedTags.AddTag(GProjectGameplayTags::State_Interaction_Pickup);
}

void UGProjectPickupItemAbility::ActivateAbility(
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
    AGProjectCharacter* Character = Cast<AGProjectCharacter>(GetAvatarActorFromActorInfo());
    UGProjectItemHolderComponent* Holder = Character ? Character->GetItemHolderComponent() : nullptr;
    if (!Holder || !Holder->HasNearbyPickup())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }
    bPickedUp = false;
    if (UCharacterMovementComponent* Move = Character->GetCharacterMovement())
    {
        Move->DisableMovement();
    }
    if (!PickupMontage)
    {
        DoPickup();
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }
    EventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
        this,
        GProjectGameplayTags::Event_Interaction_Pickup,
        nullptr,
        false,
        false);
    EventTask->EventReceived.AddDynamic(this, &ThisClass::OnPickupEvent);
    EventTask->ReadyForActivation();
    MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
        this,
        NAME_None,
        PickupMontage,
        3.0f);
    MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageFinished);
    MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageFinished);
    MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageFinished);
    MontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageFinished);
    MontageTask->ReadyForActivation();
}

void UGProjectPickupItemAbility::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility,
    bool bWasCancelled)
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

void UGProjectPickupItemAbility::OnPickupEvent(FGameplayEventData Payload)
{
    DoPickup();
}

void UGProjectPickupItemAbility::OnMontageFinished()
{
    if (!bPickedUp)
    {
        DoPickup();
    }
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGProjectPickupItemAbility::DoPickup()
{
    if (bPickedUp)
    {
        return;
    }
    bPickedUp = true;
    if (AGProjectCharacter* Character = Cast<AGProjectCharacter>(GetAvatarActorFromActorInfo()))
    {
        if (UGProjectItemHolderComponent* Holder = Character->GetItemHolderComponent())
        {
            Holder->TryPickupNearby();
        }
    }
}