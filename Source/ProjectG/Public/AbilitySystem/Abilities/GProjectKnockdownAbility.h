// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/GProjectGameplayAbility.h"
#include "GProjectKnockdownAbility.generated.h"

class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitDelay;
class UAnimMontage;

UCLASS()
class PROJECTG_API UGProjectKnockdownAbility : public UGProjectGameplayAbility
{
	GENERATED_BODY()

public:
	UGProjectKnockdownAbility();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Knockdown")
	TObjectPtr<UAnimMontage> KnockdownMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Knockdown", meta = (ClampMin = "0.1"))
	float MontagePlayRate = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Knockdown|Sections")
	FName FallSection = TEXT("Fall");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Knockdown|Sections")
	FName DownLoopSection = TEXT("DownLoop");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Knockdown|Sections")
	FName GetUpSection = TEXT("GetUp");

private:
	UFUNCTION()
	void BeginGetUp();

	UFUNCTION()
	void FinishKnockdown();

	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitDelay> GetUpDelayTask;
};
