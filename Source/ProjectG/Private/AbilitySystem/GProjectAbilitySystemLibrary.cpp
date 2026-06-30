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

void UGProjectAbilitySystemLibrary::ApplyHitstunEffect(
	const FGProjectDamageEffectParams& DamageEffectParams,
	TSubclassOf<UGameplayEffect> HitstunEffectClass)
{
	UAbilitySystemComponent* SourceASC = DamageEffectParams.SourceAbilitySystemComponent;
	UAbilitySystemComponent* TargetASC = DamageEffectParams.TargetAbilitySystemComponent;
	if (!SourceASC || !TargetASC || !HitstunEffectClass || DamageEffectParams.HitstunTime <= 0.0f)
	{
		return;
	}

	if (TargetASC->HasMatchingGameplayTag(GProjectGameplayTags::State_Character_Dead))
	{
		return;
	}

	FGameplayEffectContextHandle EffectContextHandle = SourceASC->MakeEffectContext();
	EffectContextHandle.AddSourceObject(SourceASC->GetAvatarActor());

	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(
		HitstunEffectClass,
		DamageEffectParams.AbilityLevel,
		EffectContextHandle);
	if (!SpecHandle.IsValid())
	{
		return;
	}

	SpecHandle.Data->SetDuration(DamageEffectParams.HitstunTime, true);

	FGameplayTagContainer AbilitiesToCancel;
	AbilitiesToCancel.AddTag(GProjectGameplayTags::Ability_Combat_Attack);
	TargetASC->CancelAbilities(&AbilitiesToCancel);
	TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
}

void UGProjectAbilitySystemLibrary::SendHitReactEvent(const FGProjectDamageEffectParams& DamageEffectParams)
{
	UAbilitySystemComponent* TargetASC = DamageEffectParams.TargetAbilitySystemComponent;
	if (!TargetASC || TargetASC->HasMatchingGameplayTag(GProjectGameplayTags::State_Character_Dead))
	{
		return;
	}

	AActor* TargetActor = TargetASC->GetAvatarActor();
	if (!TargetActor)
	{
		return;
	}

	FGameplayEventData EventData;
	EventData.EventTag = GProjectGameplayTags::Event_Combat_HitReact;
	EventData.Instigator = DamageEffectParams.SourceAbilitySystemComponent
		? DamageEffectParams.SourceAbilitySystemComponent->GetAvatarActor()
		: nullptr;
	EventData.Target = TargetActor;
	EventData.EventMagnitude = DamageEffectParams.HitstunTime;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		TargetActor,
		EventData.EventTag,
		EventData);
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
