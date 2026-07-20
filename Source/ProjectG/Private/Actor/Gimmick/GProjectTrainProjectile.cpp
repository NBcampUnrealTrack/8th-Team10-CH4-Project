// Copyright Epic Games, Inc. All Rights Reserved.

#include "Actor/Gimmick/GProjectTrainProjectile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"

AGProjectTrainProjectile::AGProjectTrainProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	TrainCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("TrainCollision"));
	TrainCollision->SetBoxExtent(CollisionBoxExtent);
	TrainCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TrainCollision->SetCollisionObjectType(ECC_WorldDynamic);
	TrainCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	TrainCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TrainCollision->SetGenerateOverlapEvents(true);
	SetRootComponent(TrainCollision);

	TrainMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("TrainMesh"));
	TrainMesh->SetupAttachment(RootComponent);
	TrainMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = TrainCollision;
	ProjectileMovement->ProjectileGravityScale = 0.0f;
	ProjectileMovement->bRotationFollowsVelocity = false;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->InitialSpeed = 0.0f;
	ProjectileMovement->MaxSpeed = 0.0f;
	ProjectileMovement->SetAutoActivate(false);

	LoopAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("LoopAudioComponent"));
	LoopAudioComponent->SetupAttachment(RootComponent);
	LoopAudioComponent->bAutoActivate = false;

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
}

UAbilitySystemComponent* AGProjectTrainProjectile::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AGProjectTrainProjectile::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}

	TrainCollision->SetBoxExtent(CollisionBoxExtent);

	if (HasAuthority())
	{
		TrainCollision->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnTrainBeginOverlap);
	}

	if (LoopAudioComponent && LoopSound)
	{
		LoopAudioComponent->SetSound(LoopSound);
		LoopAudioComponent->Play();
	}
}

void AGProjectTrainProjectile::InitTrain(
	const FVector& LaunchVelocity,
	const FGProjectDamageEffectParams& InDamageParams,
	TSubclassOf<UGameplayEffect> InDamageGameplayEffectClass,
	float InLifeTime)
{
	if (!HasAuthority())
	{
		return;
	}

	DamageParams = InDamageParams;
	DamageParams.SourceAbilitySystemComponent = AbilitySystemComponent;
	DamageGameplayEffectClass = InDamageGameplayEffectClass;

	ProjectileMovement->Velocity = LaunchVelocity;
	ProjectileMovement->InitialSpeed = LaunchVelocity.Size();
	ProjectileMovement->MaxSpeed = LaunchVelocity.Size();
	ProjectileMovement->Activate(true);

	if (InLifeTime > 0.0f)
	{
		SetLifeSpan(InLifeTime);
	}
}

void AGProjectTrainProjectile::OnTrainBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!HasAuthority() || !CanHitTarget(OtherActor))
	{
		return;
	}

	ApplyTrainHit(OtherActor);
}

bool AGProjectTrainProjectile::CanHitTarget(AActor* Target) const
{
	if (!Target || Target == this)
	{
		return false;
	}

	return UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target) != nullptr;
}

void AGProjectTrainProjectile::ApplyTrainHit(AActor* Target)
{
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
	if (!TargetASC || !DamageParams.SourceAbilitySystemComponent || !DamageGameplayEffectClass)
	{
		return;
	}

	FGProjectDamageEffectParams HitDamageParams = DamageParams;
	HitDamageParams.TargetAbilitySystemComponent = TargetASC;

	FVector KnockbackDirection = GetActorForwardVector();
	KnockbackDirection.Z = 0.0f;
	if (!KnockbackDirection.IsNearlyZero())
	{
		KnockbackDirection.Normalize();
		HitDamageParams.HitDirection = KnockbackDirection;
		HitDamageParams.KnockbackForce = KnockbackDirection * HitDamageParams.KnockbackForceMagnitude;
	}

	UGProjectAbilitySystemLibrary::ApplyDamageEffect(
		HitDamageParams,
		DamageGameplayEffectClass);

	if (HitDamageParams.CausesKnockdown())
	{
		UGProjectAbilitySystemLibrary::SendKnockdownEvent(HitDamageParams);
	}

	if (ACharacter* TargetCharacter = Cast<ACharacter>(Target);
		TargetCharacter && !HitDamageParams.KnockbackForce.IsNearlyZero())
	{
		TargetCharacter->LaunchCharacter(HitDamageParams.KnockbackForce, true, true);
	}
}
