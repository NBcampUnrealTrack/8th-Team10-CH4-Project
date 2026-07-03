#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/GProjectGameplayAbility.h"
#include "GProjectUseItemAbility.generated.h"

UCLASS()
class PROJECTG_API UGProjectUseItemAbility : public UGProjectGameplayAbility
{
    GENERATED_BODY()

public:
    UGProjectUseItemAbility();

protected:
    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData) override;
};