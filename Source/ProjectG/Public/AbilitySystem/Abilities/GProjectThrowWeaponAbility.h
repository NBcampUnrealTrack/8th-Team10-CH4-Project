#pragma once

#include "AbilitySystem/Abilities/GProjectGameplayAbility.h"
#include "AbilitySystem/GProjectAbilitySystemLibrary.h"
#include "CoreMinimal.h"
#include "GProjectThrowWeaponAbility.generated.h"

class AGProjectThrownWeapon;
class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;
class UAnimMontage;
class UGameplayEffect;
struct FGameplayEventData;

UCLASS()
class PROJECTG_API UGProjectThrowWeaponAbility : public UGProjectGameplayAbility
{
	GENERATED_BODY()

public:
	UGProjectThrowWeaponAbility();

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

	UPROPERTY(EditDefaultsOnly, Category = "Throw")
	TObjectPtr<UAnimMontage> ThrowMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Throw")
	FName ThrowSocketName = TEXT("hand_r");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Throw|Tuning", meta = (ClampMin = "0.0"))
	float ThrowSpawnForwardOffset = 80.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Throw|Tuning", meta = (ClampMin = "0.0"))
	float ThrowSpeed = 1500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Throw|Tuning", meta = (ClampMin = "0.0"))
	float ThrowGravityScale = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Throw|Tuning", meta = (ClampMin = "0.1"))
	float MaxFlightTime = 5.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Throw|Damage")
	FGProjectDamageEffectParams ThrowDamageParams;

	UPROPERTY(EditDefaultsOnly, Category = "Throw|Damage")
	TSubclassOf<UGameplayEffect> DamageGameplayEffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "Throw|Damage")
	TSubclassOf<UGameplayEffect> HitstunGameplayEffectClass;

private:
	UFUNCTION()
	void OnThrowReleaseEvent(FGameplayEventData Payload);

	UFUNCTION()
	void OnMontageEnded();

	void ThrowHeldWeapon();
	void FinishAbility();

	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> EventTask;

	bool bWeaponThrown = false;
};
