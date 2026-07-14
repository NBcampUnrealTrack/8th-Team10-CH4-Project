// Copyright Epic Games, Inc. All Rights Reserved.

#include "Actor/GProjectBombActor.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/DecalComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GProjectGameplayTags.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystem.h"

AGProjectBombActor::AGProjectBombActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	BombMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BombMesh"));
	BombMesh->SetupAttachment(SceneRoot);
	BombMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	WarningDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("WarningDecal"));
	WarningDecal->SetupAttachment(SceneRoot);
	WarningDecal->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));
	WarningDecal->DecalSize = FVector(DecalProjectionDepth, 0.0f, 0.0f);
	WarningDecal->SetVisibility(false);
}

void AGProjectBombActor::BeginPlay()
{
	Super::BeginPlay();

	if (BombMesh)
	{
		BombMaterial = BombMesh->CreateAndSetMaterialInstanceDynamic(0);
	}

	if (WarningDecal && WarningDecalMaterial)
	{
		WarningDecal->SetDecalMaterial(WarningDecalMaterial);
	}

	if (BombMaterial)
	{
		BombMaterial->SetScalarParameterValue(BombFlashParameterName, 0.0f);
	}

	if (WarningDecal)
	{
		WarningDecal->SetVisibility(bFuseStarted);
	}

	UpdateWarningDecal();
}

void AGProjectBombActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bFuseStarted || bExploded)
	{
		return;
	}

	ElapsedTime += DeltaSeconds;
	UpdateWarningDecal();
	UpdateBombBlink(DeltaSeconds);

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
	StartFuseVisuals();
	ForceNetUpdate();
}

void AGProjectBombActor::OnRep_FuseStarted()
{
	if (bFuseStarted)
	{
		StartFuseVisuals();
	}
}

void AGProjectBombActor::StartFuseVisuals()
{
	ElapsedTime = 0.0f;
	BlinkElapsedTime = 0.0f;
	bBlinkOn = false;

	if (BombMaterial)
	{
		BombMaterial->SetScalarParameterValue(BombFlashParameterName, 0.0f);
	}

	if (WarningDecal)
	{
		WarningDecal->SetVisibility(true);
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
	const float CurrentRadius = FMath::Lerp(0.0f, ExplosionRadius, Alpha);
	WarningDecal->DecalSize = FVector(DecalProjectionDepth, CurrentRadius, CurrentRadius);
}

void AGProjectBombActor::UpdateBombBlink(float DeltaSeconds)
{
	if (!BombMaterial)
	{
		return;
	}

	const float Alpha = FuseTime > 0.0f ? FMath::Clamp(ElapsedTime / FuseTime, 0.0f, 1.0f) : 1.0f;
	const float BlinkInterval = FMath::Lerp(MaxBlinkInterval, MinBlinkInterval, Alpha);

	BlinkElapsedTime += DeltaSeconds;
	if (BlinkElapsedTime < BlinkInterval)
	{
		return;
	}

	BlinkElapsedTime = 0.0f;
	bBlinkOn = !bBlinkOn;

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
			GetActorRotation());
	}
}
