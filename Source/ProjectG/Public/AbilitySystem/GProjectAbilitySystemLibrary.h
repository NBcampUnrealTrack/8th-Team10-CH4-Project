// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/GProjectCombatFeedbackTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayEffectTypes.h"
#include "GProjectAbilitySystemLibrary.generated.h"

class UAbilitySystemComponent;
class UGameplayEffect;

USTRUCT(BlueprintType)
struct FGProjectDamageEffectParams
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Damage")
	TObjectPtr<UAbilitySystemComponent> SourceAbilitySystemComponent = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Damage")
	TObjectPtr<UAbilitySystemComponent> TargetAbilitySystemComponent = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Damage")
	float BaseDamage = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Hit Reaction")
	FVector HitDirection = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "Hit Reaction")
	FVector KnockbackForce = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Hit Reaction")
	float KnockbackForceMagnitude = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Airborne", meta = (ClampMin = "0.0"))
	float AirborneLaunchForce = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Hit Reaction")
	float HitstunTime = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Hit Reaction", meta = (ClampMin = "0.0"))
	float KnockdownGroundTime = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Feedback")
	EGProjectCameraShakeLevel CameraShakeLevel = EGProjectCameraShakeLevel::None;

	bool CausesKnockdown() const
	{
		return KnockdownGroundTime > 0.0f;
	}
};

UCLASS()
class PROJECTG_API UGProjectAbilitySystemLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "ProjectG|AbilitySystem")
	static FGameplayEffectContextHandle ApplyDamageEffect(
		const FGProjectDamageEffectParams& DamageEffectParams,
		TSubclassOf<UGameplayEffect> DamageEffectClass);

	UFUNCTION(BlueprintCallable, Category = "ProjectG|AbilitySystem")
	static void ApplyHitstunEffect(
		const FGProjectDamageEffectParams& DamageEffectParams,
		TSubclassOf<UGameplayEffect> HitstunEffectClass);

	UFUNCTION(BlueprintCallable, Category = "ProjectG|AbilitySystem")
	static void SendHitReactEvent(const FGProjectDamageEffectParams& DamageEffectParams);

	UFUNCTION(BlueprintCallable, Category = "ProjectG|AbilitySystem")
	static void SendKnockdownEvent(const FGProjectDamageEffectParams& DamageEffectParams);

	UFUNCTION(BlueprintCallable, Category = "ProjectG|AbilitySystem")
	static void SetKnockbackDirection(FGProjectDamageEffectParams& DamageEffectParams, FVector KnockbackDirection, float Magnitude = 0.0f);

	UFUNCTION(BlueprintCallable, Category = "ProjectG|Feedback")
	static void PlayCameraShakeFeedback(const FGProjectDamageEffectParams& DamageEffectParams);

	UFUNCTION(BlueprintPure, Category = "ProjectG|Combat")
	static bool IsActorInFrontArc(const AActor* Defender, const AActor* Attacker, float ArcAngleDegrees);

	UFUNCTION(BlueprintCallable, Category = "ProjectG|AbilitySystem", meta = (WorldContext = "WorldContextObject"))
	static void GetLivePlayersWithinRadius(const UObject* WorldContextObject, TArray<AActor*>& OutOverlappingActors, const TArray<AActor*>& ActorsToIgnore, float Radius, const FVector& SphereOrigin);

	UFUNCTION(BlueprintCallable, Category = "ProjectG|AbilitySystem")
	static void GetClosestTargets(int32 MaxTargets, const TArray<AActor*>& Actors, TArray<AActor*>& OutClosestTargets, const FVector& Origin);
};
