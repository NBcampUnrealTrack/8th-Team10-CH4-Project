// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/GProjectGameplayAbility.h"
#include "AbilitySystem/GProjectAbilitySystemLibrary.h"
#include "GProjectAttackComboAbility.generated.h"

class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;
class UAnimMontage;
class UGameplayEffect;

UENUM(BlueprintType)
enum class EGProjectAttackInput : uint8
{
	Basic,
	Strong
};

USTRUCT(BlueprintType)
struct FGProjectComboStep
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
	TArray<EGProjectAttackInput> InputSequence;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
	FName MontageSection;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
	FGProjectDamageEffectParams DamageParams;
};

struct FGProjectBufferedAttackInput
{
	EGProjectAttackInput Input = EGProjectAttackInput::Basic;
	double ExpireTime = 0.0;
};

UCLASS()
class PROJECTG_API UGProjectAttackComboAbility : public UGProjectGameplayAbility
{
	GENERATED_BODY()

public:
	UGProjectAttackComboAbility();

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
	TObjectPtr<UAnimMontage> ComboMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
	TArray<FGProjectComboStep> ComboSteps;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo", meta = (ClampMin = "0.05"))
	float InputBufferLifetime = 1.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo", meta = (ClampMin = "1"))
	int32 MaxBufferedInputs = 8;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit Detection", meta = (ClampMin = "0.0"))
	float AttackRange = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit Detection", meta = (ClampMin = "0.0"))
	float AttackRadius = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit Reaction")
	TSubclassOf<UGameplayEffect> HitstunGameplayEffectClass;

private:
	UFUNCTION()
	void OnGameplayEvent(FGameplayEventData Payload);

	UFUNCTION()
	void OnMontageEnded();

	void HandleAttackInput(EGProjectAttackInput AttackInput);
	void StartCombo(EGProjectAttackInput AttackInput);
	void AddBufferedInput(EGProjectAttackInput AttackInput);
	void RemoveExpiredInputs();
	void TryReserveNextSection();
	void SyncCurrentStepFromMontage();
	void ApplyCurrentStepHit();
	
	int32 FindComboStepIndex(const TArray<EGProjectAttackInput>& InputSequence) const;
	int32 FindComboStepIndexBySection(FName MontageSection) const;
	
	const FGProjectComboStep* GetCurrentComboStep() const;
	
	void FinishAbility();

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> EventTask;

	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

	TArray<FGProjectBufferedAttackInput> InputBuffer;
	TSet<TWeakObjectPtr<AActor>> HitActorsThisStep;
	int32 CurrentComboStepIndex = INDEX_NONE;
	bool bComboWindowOpen = false;
	bool bNextSectionReserved = false;
};
