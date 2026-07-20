// Copyright Epic Games, Inc. All Rights Reserved.

#include "Actor/Gimmick/GProjectTrainSpawner.h"

#include "Actor/Gimmick/GProjectTrainProjectile.h"
#include "Components/ArrowComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

AGProjectTrainSpawner::AGProjectTrainSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	SceneRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRootComponent"));
	SetRootComponent(SceneRootComponent);

	SpawnDirection = CreateDefaultSubobject<UArrowComponent>(TEXT("SpawnDirection"));
	SpawnDirection->SetupAttachment(RootComponent);
}

void AGProjectTrainSpawner::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		StartSpawnCycle();
	}
}

void AGProjectTrainSpawner::StartSpawnCycle()
{
	if (!HasAuthority())
	{
		return;
	}

	MulticastPlayWarningSound();

	const float SpawnDelay = FMath::Max(WarningLeadTime, 0.0f);
	GetWorldTimerManager().SetTimer(
		SpawnTimerHandle,
		this,
		&ThisClass::SpawnTrain,
		SpawnDelay,
		false);

	const float NextCycleDelay = FMath::Max(SpawnInterval, SpawnDelay + 0.1f);
	GetWorldTimerManager().SetTimer(
		CycleTimerHandle,
		this,
		&ThisClass::StartSpawnCycle,
		NextCycleDelay,
		false);
}

void AGProjectTrainSpawner::SpawnTrain()
{
	if (!HasAuthority() || !TrainClass)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetInstigator();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AGProjectTrainProjectile* Train = World->SpawnActor<AGProjectTrainProjectile>(
		TrainClass,
		GetActorLocation(),
		GetActorRotation(),
		SpawnParams);
	if (!Train)
	{
		return;
	}

	FGProjectDamageEffectParams TrainDamageParams = DamageParams;

	const FVector LaunchVelocity = GetActorForwardVector() * TrainSpeed;
	Train->InitTrain(
		LaunchVelocity,
		TrainDamageParams,
		DamageGameplayEffectClass,
		TrainLifeTime);
	Train->ForceNetUpdate();
}

void AGProjectTrainSpawner::MulticastPlayWarningSound_Implementation()
{
	if (!WarningSound)
	{
		return;
	}

	UGameplayStatics::PlaySoundAtLocation(
		this,
		WarningSound,
		GetActorLocation(),
		WarningSoundVolume);
}
