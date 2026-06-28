// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/GProjectAbilitySystemLibrary.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GProjectGameplayTags.h"
#include "CollisionQueryParams.h"
#include "Engine/Engine.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"

FGameplayEffectContextHandle UGProjectAbilitySystemLibrary::ApplyDamageEffect(const FGProjectDamageEffectParams& DamageEffectParams)
{
	if (!DamageEffectParams.SourceAbilitySystemComponent || !DamageEffectParams.TargetAbilitySystemComponent || !DamageEffectParams.DamageGameplayEffectClass)
	{
		return FGameplayEffectContextHandle();
	}

	const AActor* SourceAvatarActor = DamageEffectParams.SourceAbilitySystemComponent->GetAvatarActor();

	FGameplayEffectContextHandle EffectContextHandle = DamageEffectParams.SourceAbilitySystemComponent->MakeEffectContext();
	EffectContextHandle.AddSourceObject(SourceAvatarActor);

	const FGameplayEffectSpecHandle SpecHandle = DamageEffectParams.SourceAbilitySystemComponent->MakeOutgoingSpec(
		DamageEffectParams.DamageGameplayEffectClass,
		DamageEffectParams.AbilityLevel,
		EffectContextHandle
	);

	if (SpecHandle.IsValid())
	{
		UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GProjectGameplayTags::Data_Combat_Damage, DamageEffectParams.BaseDamage);
		DamageEffectParams.TargetAbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}

	return EffectContextHandle;
}

void UGProjectAbilitySystemLibrary::SetKnockbackDirection(FGProjectDamageEffectParams& DamageEffectParams, FVector KnockbackDirection, float Magnitude)
{
	KnockbackDirection.Normalize();

	const float KnockbackMagnitude = Magnitude == 0.0f ? DamageEffectParams.KnockbackForceMagnitude : Magnitude;
	DamageEffectParams.KnockbackForce = KnockbackDirection * KnockbackMagnitude;
}

void UGProjectAbilitySystemLibrary::GetLivePlayersWithinRadius(
	const UObject* WorldContextObject,
	TArray<AActor*>& OutOverlappingActors,
	const TArray<AActor*>& ActorsToIgnore,
	float Radius,
	const FVector& SphereOrigin)
{
	OutOverlappingActors.Reset();

	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
	if (!World)
	{
		return;
	}

	FCollisionQueryParams SphereParams(SCENE_QUERY_STAT(GetLivePlayersWithinRadius), false);
	SphereParams.AddIgnoredActors(ActorsToIgnore);

	TArray<FOverlapResult> Overlaps;
	World->OverlapMultiByObjectType(
		Overlaps,
		SphereOrigin,
		FQuat::Identity,
		FCollisionObjectQueryParams(FCollisionObjectQueryParams::InitType::AllDynamicObjects),
		FCollisionShape::MakeSphere(Radius),
		SphereParams
	);

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* OverlapActor = Overlap.GetActor();
		if (!OverlapActor)
		{
			continue;
		}

		UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OverlapActor);
		if (!ASC || ASC->HasMatchingGameplayTag(GProjectGameplayTags::State_Character_Dead))
		{
			continue;
		}

		OutOverlappingActors.AddUnique(OverlapActor);
	}
}

void UGProjectAbilitySystemLibrary::GetClosestTargets(int32 MaxTargets, const TArray<AActor*>& Actors, TArray<AActor*>& OutClosestTargets, const FVector& Origin)
{
	OutClosestTargets.Reset();

	if (MaxTargets <= 0)
	{
		return;
	}

	if (Actors.Num() <= MaxTargets)
	{
		OutClosestTargets = Actors;
		return;
	}

	TArray<AActor*> ActorsToCheck = Actors;

	while (OutClosestTargets.Num() < MaxTargets && ActorsToCheck.Num() > 0)
	{
		double ClosestDistanceSquared = TNumericLimits<double>::Max();
		AActor* ClosestActor = nullptr;

		for (AActor* PotentialTarget : ActorsToCheck)
		{
			if (!PotentialTarget)
			{
				continue;
			}

			const double DistanceSquared = FVector::DistSquared(PotentialTarget->GetActorLocation(), Origin);
			if (DistanceSquared < ClosestDistanceSquared)
			{
				ClosestDistanceSquared = DistanceSquared;
				ClosestActor = PotentialTarget;
			}
		}

		if (!ClosestActor)
		{
			break;
		}

		ActorsToCheck.Remove(ClosestActor);
		OutClosestTargets.AddUnique(ClosestActor);
	}
}
