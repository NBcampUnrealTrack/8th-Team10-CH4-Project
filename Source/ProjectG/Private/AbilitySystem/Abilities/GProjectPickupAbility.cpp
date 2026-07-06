#include "AbilitySystem/Abilities/GProjectPickupAbility.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Item/GItemHolderComponent.h"
#include "GameFramework/Character.h"
#include "GProjectGameplayTags.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"

UGProjectPickupAbility::UGProjectPickupAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

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

    bPickedUp = false;

    if (ACharacter* Char = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
    {
        if (UCharacterMovementComponent* Move = Char->GetCharacterMovement())
        {
            Move->DisableMovement();
        }
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

void UGProjectPickupAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
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

void UGProjectPickupAbility::OnPickupEvent(FGameplayEventData Payload)
{
    DoPickup();
}

void UGProjectPickupAbility::OnMontageFinished()
{
    if (!bPickedUp)
    {
        DoPickup();
    }

    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGProjectPickupAbility::DoPickup()
{
    if (bPickedUp)
    {
        return;
    }
    bPickedUp = true;

    if (AActor* Avatar = GetAvatarActorFromActorInfo())
    {
        if (UGItemHolderComponent* Holder = Avatar->FindComponentByClass<UGItemHolderComponent>())
        {
            Holder->TryPickupNearby();
        }
    }
}