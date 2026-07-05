#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/GProjectGameplayAbility.h"
#include "GProjectPickupAbility.generated.h"

class UAnimMontage;
class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;
struct FGameplayEventData;

UCLASS()
class PROJECTG_API UGProjectPickupAbility : public UGProjectGameplayAbility
{
    GENERATED_BODY()

public:
    UGProjectPickupAbility();

protected:
    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData) override;

    UPROPERTY(EditDefaultsOnly, Category = "Pickup")
    TObjectPtr<UAnimMontage> PickupMontage;

private:
    UFUNCTION()
    void OnMontageFinished();

    UFUNCTION()
    void OnPickupEvent(FGameplayEventData Payload);

    void DoPickup();

    UPROPERTY()
    TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

    UPROPERTY()
    TObjectPtr<UAbilityTask_WaitGameplayEvent> EventTask;

    bool bPickedUp = false;
};