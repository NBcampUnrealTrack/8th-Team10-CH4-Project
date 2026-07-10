// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/GProjectGameplayAbility.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "GProjectParryAbility.generated.h"

class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;
class UAnimMontage;

UCLASS()
class PROJECTG_API UGProjectParryAbility : public UGProjectGameplayAbility
{
	GENERATED_BODY()

public:
	UGProjectParryAbility();

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Parry|Animation")
	TObjectPtr<UAnimMontage> ParryMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Parry|Animation", meta = (ClampMin = "0.1"))
	float MontagePlayRate = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Parry|Animation")
	FName BlockStartSection = TEXT("Section_BlockStart");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Parry|Animation")
	FName LoopSection = TEXT("Section_Loop");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Parry|Animation")
	FName EndSection = TEXT("Section_End");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Parry|Animation")
	FName SuccessSection = TEXT("Section_Hit");

private:
	UFUNCTION()
	void OnParryWindowClosed(FGameplayEventData Payload);

	UFUNCTION()
	void OnParrySuccess(FGameplayEventData Payload);

	UFUNCTION()
	void OnMontageFinished();

	void CloseParryWindow() const;
	void ConfigureParrySectionLinks() const;
	bool JumpToParrySection(FName SectionName) const;
	void FinishParry(bool bWasCancelled = false);

	bool bFinishingParry = false;

	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> ParrySuccessTask;

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> ParryWindowClosedTask;
};
