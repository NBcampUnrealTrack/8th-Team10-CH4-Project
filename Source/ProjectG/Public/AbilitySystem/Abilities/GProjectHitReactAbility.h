// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/GProjectGameplayAbility.h"
#include "GProjectHitReactAbility.generated.h"

class UAbilityTask_PlayMontageAndWait;
class UAnimMontage;
class USoundBase;

UCLASS()
class PROJECTG_API UGProjectHitReactAbility : public UGProjectGameplayAbility
{
	GENERATED_BODY()

public:
	UGProjectHitReactAbility();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit Reaction")
	TObjectPtr<UAnimMontage> HitReactMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit Reaction|Sections")
	FName FrontSection = TEXT("HitFront");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit Reaction|Sections")
	FName BackSection = TEXT("HitBack");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit Reaction|Sections")
	FName LeftSection = TEXT("HitLeft");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit Reaction|Sections")
	FName RightSection = TEXT("HitRight");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit Reaction|Sections")
	FName ParriedSection = TEXT("Parried");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit Reaction|Sound")
	TObjectPtr<USoundBase> HitReactSound;

private:
	FName DetermineHitSection(const FGameplayEventData* TriggerEventData) const;

	UFUNCTION()
	void FinishHitReact();

	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;
};
