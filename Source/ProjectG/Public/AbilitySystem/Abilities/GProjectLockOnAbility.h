// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/GProjectGameplayAbility.h"
#include "GameplayEffectTypes.h"
#include "GProjectLockOnAbility.generated.h"

class AActor;
class UAbilityTask_WaitAttributeChangeThreshold;
class UGameplayEffect;

UCLASS()
class PROJECTG_API UGProjectLockOnAbility : public UGProjectGameplayAbility
{
	GENERATED_BODY()

public:
	UGProjectLockOnAbility();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

	virtual void InputReleased(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) override;

	virtual bool CheckCost(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cost")
	TSubclassOf<UGameplayEffect> LockOnDrainGameplayEffectClass;

private:
	UFUNCTION()
	void OnSPThresholdReached(bool bMatchesComparison, float CurrentValue);

	UFUNCTION()
	void OnLockOnTargetChanged(AActor* NewTarget);

	void FinishLockOn();

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitAttributeChangeThreshold> SPThresholdTask;

	FActiveGameplayEffectHandle LockOnDrainEffectHandle;
};
