// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/GProjectGameplayAbility.h"
#include "AbilitySystem/Combo/GProjectComboTypes.h"
#include "GProjectAttackComboAbility.generated.h"

class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;
class UAnimMontage;
class UGameplayEffect;
class UGProjectComboData;

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	TSubclassOf<UGameplayEffect> DamageGameplayEffectClass;

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
	void BeginCurrentStepTrace(FName TraceSocketName);
	void EndCurrentStepTrace();
	void ApplyCurrentStepHit();
	
	int32 FindComboStepIndex(const TArray<EGProjectAttackInput>& InputSequence) const;
	int32 FindComboStepIndexBySection(FName MontageSection) const;
	
	const FGProjectComboStep* GetCurrentComboStep() const;
	
	void FinishAbility();

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> EventTask;

	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

	UPROPERTY()
	TObjectPtr<UGProjectComboData> ComboData;

	TArray<FGProjectBufferedAttackInput> InputBuffer;
	TSet<TWeakObjectPtr<AActor>> HitActorsThisStep;
	int32 CurrentComboStepIndex = INDEX_NONE;
	bool bComboWindowOpen = false;
	bool bNextSectionReserved = false;
	bool bHasPreviousUnarmedTraceLocation = false;
	FVector PreviousUnarmedTraceLocation = FVector::ZeroVector;
	FName CurrentUnarmedTraceSocket = NAME_None;
};
