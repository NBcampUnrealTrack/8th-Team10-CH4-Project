// Copyright Epic Games, Inc. All Rights Reserved.

#include "Actor/GProjectBombActor.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Character/GProjectCharacter.h"
#include "Components/DecalComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GProjectGameplayTags.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystem.h"

namespace
{
	const FName FuseProgressParameterName = TEXT("FuseProgress");
	const FName BombFlashParameterName = TEXT("BombFlashAmount");

	constexpr float DecalProjectionDepth = 256.0f;
	constexpr float WarningDecalGroundTraceUp = 100.0f;
	constexpr float WarningDecalGroundTraceDown = 1000.0f;
	constexpr float WarningDecalGroundOffset = 3.0f;
	constexpr float MaxBlinkInterval = 0.5f;
	constexpr float MinBlinkInterval = 0.08f;
	constexpr float BombFlashAmount = 1.0f;
}

AGProjectBombActor::AGProjectBombActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	WarningDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("WarningDecal"));
	WarningDecal->SetupAttachment(RootComponent);
	WarningDecal->SetUsingAbsoluteScale(true);
	WarningDecal->SetWorldRotation(FRotator(-90.0f, 0.0f, 0.0f));
	WarningDecal->SetWorldScale3D(FVector::OneVector);
	WarningDecal->DecalSize = FVector(DecalProjectionDepth, 1.0f, 1.0f);
	WarningDecal->SetVisibility(false);
	WarningDecal->SetHiddenInGame(true);
}

void AGProjectBombActor::BeginPlay()
{
	Super::BeginPlay();

	if (ItemMesh)
	{
		BombMaterial = ItemMesh->CreateAndSetMaterialInstanceDynamic(0);
	}

	if (WarningDecal && WarningDecalMaterial)
	{
		WarningDecal->SetDecalMaterial(WarningDecalMaterial);
		WarningDecalMaterialInstance = WarningDecal->CreateDynamicMaterialInstance();
		if (WarningDecalMaterialInstance)
		{
			WarningDecalMaterialInstance->SetScalarParameterValue(FuseProgressParameterName, 0.0f);
		}
	}

	if (BombMaterial)
	{
		BombMaterial->SetScalarParameterValue(BombFlashParameterName, 0.0f);
	}

	if (WarningDecal)
	{
		WarningDecal->SetVisibility(bFuseStarted, true);
		WarningDecal->SetHiddenInGame(!bFuseStarted, true);
	}

	if (bFuseStarted)
	{
		SetActorTickEnabled(true);
		UpdateWarningDecal();
	}
}

void AGProjectBombActor::Tick(float DeltaSeconds)
{
	if (!bFuseStarted)
	{
		Super::Tick(DeltaSeconds);
		return;
	}

	if (bExploded)
	{
		return;
	}

	ElapsedTime += DeltaSeconds;

	UpdateWarningDecal();
	UpdateBombBlink();

	if (HasAuthority() && ElapsedTime >= FuseTime)
	{
		Explode();
	}
}

void AGProjectBombActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGProjectBombActor, bFuseStarted);
}

bool AGProjectBombActor::ShouldDestroyOnUse() const
{
	return false;
}

bool AGProjectBombActor::ShouldApplyThrowImpactDamage() const
{
	return false;
}

void AGProjectBombActor::OnThrowStarted(AGProjectCharacter* Thrower)
{
	if (!HasAuthority() || bFuseStarted || bExploded)
	{
		return;
	}

	SetSourceActor(Thrower);
	StartFuse();
}

bool AGProjectBombActor::Use_Implementation(AGProjectCharacter* Character)
{
	if (!HasAuthority() || bFuseStarted || bExploded)
	{
		return false;
	}

	SetSourceActor(Character);
	StartFuse();
	return true;
}

void AGProjectBombActor::SetSourceActor(AActor* InSourceActor)
{
	SourceActor = InSourceActor;
}

void AGProjectBombActor::StartFuse()
{
	if (bFuseStarted || bExploded)
	{
		return;
	}

	if (!HasAuthority())
	{
		return;
	}

	bFuseStarted = true;
	SetActorTickEnabled(true);
	StartFuseVisuals();
	ForceNetUpdate();
}

void AGProjectBombActor::OnRep_FuseStarted()
{
	if (bFuseStarted)
	{
		SetActorTickEnabled(true);
		StartFuseVisuals();
	}
}

void AGProjectBombActor::StartFuseVisuals()
{
	ElapsedTime = 0.0f;

	if (BombMaterial)
	{
		BombMaterial->SetScalarParameterValue(BombFlashParameterName, 0.0f);
	}

	if (WarningDecal)
	{
		WarningDecal->SetVisibility(true, true);
		WarningDecal->SetHiddenInGame(false, true);
	}

	UpdateWarningDecal();
}

void AGProjectBombActor::UpdateWarningDecal()
{
	if (!WarningDecal)
	{
		return;
	}

	const float Alpha = FuseTime > 0.0f ? FMath::Clamp(ElapsedTime / FuseTime, 0.0f, 1.0f) : 1.0f;
	const float MaxDiameter = FMath::Max(ExplosionRadius * 2.0f, 1.0f);
	WarningDecal->SetWorldScale3D(FVector::OneVector);
	WarningDecal->DecalSize = FVector(DecalProjectionDepth, MaxDiameter, MaxDiameter);
	WarningDecal->SetWorldRotation(FRotator(-90.0f, 0.0f, 0.0f));

	if (WarningDecalMaterialInstance)
	{
		WarningDecalMaterialInstance->SetScalarParameterValue(FuseProgressParameterName, Alpha);
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FVector TraceStart = GetActorLocation() + FVector(0.0f, 0.0f, WarningDecalGroundTraceUp);
	const FVector TraceEnd = GetActorLocation() - FVector(0.0f, 0.0f, WarningDecalGroundTraceDown);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(BombWarningDecalGroundTrace), false, this);
	QueryParams.AddIgnoredActor(this);

	FHitResult GroundHit;
	if (World->LineTraceSingleByChannel(GroundHit, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
	{
		WarningDecal->SetWorldLocation(GroundHit.ImpactPoint + FVector(0.0f, 0.0f, WarningDecalGroundOffset));
	}
	else
	{
		WarningDecal->SetWorldLocation(GetActorLocation());
	}
}

void AGProjectBombActor::UpdateBombBlink()
{
	if (!BombMaterial)
	{
		return;
	}

	const float Alpha = FuseTime > 0.0f ? FMath::Clamp(ElapsedTime / FuseTime, 0.0f, 1.0f) : 1.0f;
	const float BlinkInterval = FMath::Lerp(MaxBlinkInterval, MinBlinkInterval, Alpha);
	const float BlinkPhase = FMath::Fmod(ElapsedTime, BlinkInterval);
	const bool bBlinkOn = BlinkPhase < BlinkInterval * 0.5f;

	BombMaterial->SetScalarParameterValue(
		BombFlashParameterName,
		bBlinkOn ? BombFlashAmount : 0.0f);
}

void AGProjectBombActor::Explode()
{
	if (!HasAuthority() || bExploded)
	{
		return;
	}

	bExploded = true;
	MulticastPlayExplosionFeedback();

	TArray<AActor*> Targets;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);

	UGProjectAbilitySystemLibrary::GetLivePlayersWithinRadius(
		this,
		Targets,
		ActorsToIgnore,
		ExplosionRadius,
		GetActorLocation());

	for (AActor* Target : Targets)
	{
		ApplyExplosionToTarget(Target);
	}

	Destroy();
}

void AGProjectBombActor::ApplyExplosionToTarget(AActor* TargetActor)
{
	if (!TargetActor)
	{
		return;
	}

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (!TargetASC)
	{
		return;
	}

	UAbilitySystemComponent* SourceASC = GetSourceAbilitySystemComponent();
	if (!SourceASC)
	{
		SourceASC = TargetASC;
	}

	FGProjectDamageEffectParams DamageParams = ExplosionDamageParams;
	DamageParams.SourceAbilitySystemComponent = SourceASC;
	DamageParams.TargetAbilitySystemComponent = TargetASC;

	FVector KnockbackDirection = TargetActor->GetActorLocation() - GetActorLocation();
	KnockbackDirection.Z = 0.0f;
	UGProjectAbilitySystemLibrary::SetKnockbackDirection(DamageParams, KnockbackDirection);

	const FGameplayEffectContextHandle DamageContext = UGProjectAbilitySystemLibrary::ApplyDamageEffect(
		DamageParams,
		DamageGameplayEffectClass);
	if (!DamageContext.IsValid())
	{
		return;
	}

	if (DamageParams.CausesKnockdown())
	{
		UGProjectAbilitySystemLibrary::SendKnockdownEvent(DamageParams);
	}
	else
	{
		UGProjectAbilitySystemLibrary::ApplyHitstunEffect(DamageParams, HitstunGameplayEffectClass);
		UGProjectAbilitySystemLibrary::SendHitReactEvent(DamageParams);
	}

	ACharacter* TargetCharacter = Cast<ACharacter>(TargetActor);
	if (!TargetCharacter)
	{
		return;
	}

	if (!DamageParams.KnockbackForce.IsNearlyZero() || DamageParams.AirborneLaunchForce > 0.0f)
	{
		FVector LaunchVelocity = DamageParams.KnockbackForce;
		LaunchVelocity.Z = DamageParams.AirborneLaunchForce > 0.0f ? DamageParams.AirborneLaunchForce : 200.0f;

		if (DamageParams.AirborneLaunchForce > 0.0f)
		{
			TargetASC->AddLooseGameplayTag(GProjectGameplayTags::State_Combat_Airborne);
		}

		TargetCharacter->LaunchCharacter(LaunchVelocity, true, true);
	}
}

UAbilitySystemComponent* AGProjectBombActor::GetSourceAbilitySystemComponent() const
{
	return SourceActor
		? UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(SourceActor)
		: nullptr;
}

void AGProjectBombActor::MulticastPlayExplosionFeedback_Implementation()
{
	if (ExplosionSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ExplosionSound, GetActorLocation());
	}

	if (ExplosionEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			this,
			ExplosionEffect,
			GetActorLocation(),
			GetActorRotation(),
			ExplosionEffectScale);
	}
}
