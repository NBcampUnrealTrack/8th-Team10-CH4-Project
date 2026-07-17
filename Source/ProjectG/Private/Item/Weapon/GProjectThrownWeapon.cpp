#include "Item/Weapon/GProjectThrownWeapon.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Item/GProjectItemActorBase.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

AGProjectThrownWeapon::AGProjectThrownWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	ThrowCollision = CreateDefaultSubobject<USphereComponent>(TEXT("ThrowCollision"));
	ThrowCollision->InitSphereRadius(CollisionRadius);
	ThrowCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ThrowCollision->SetCollisionObjectType(ECC_WorldDynamic);
	ThrowCollision->SetCollisionResponseToAllChannels(ECR_Block);
	ThrowCollision->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	ThrowCollision->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	SetRootComponent(ThrowCollision);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->SetUpdatedComponent(ThrowCollision);
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->bAutoActivate = false;
	ProjectileMovement->ProjectileGravityScale = 1.0f;
}

void AGProjectThrownWeapon::BeginPlay()
{
	Super::BeginPlay();

	ThrowCollision->SetSphereRadius(CollisionRadius);

	if (HasAuthority())
	{
		ThrowCollision->OnComponentHit.AddDynamic(this, &ThisClass::OnThrowCollisionHit);
		return;
	}

	ThrowCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AGProjectThrownWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGProjectThrownWeapon, CarriedItem);
}

void AGProjectThrownWeapon::OnRep_CarriedItem()
{
	AttachCarriedItemVisual();
}

void AGProjectThrownWeapon::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HasAuthority() && CarriedItem)
	{
		ReleaseCarriedItem(GetActorLocation());
	}
	else if (!HasAuthority() && CarriedItem)
	{
		CarriedItem->DetachItemVisual();
		CarriedItem = nullptr;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FlightTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void AGProjectThrownWeapon::InitAndLaunch(
	AGProjectItemActorBase* WeaponItem,
	const FVector& LaunchVelocity,
	const FGProjectDamageEffectParams& InDamageParams,
	TSubclassOf<UGameplayEffect> InDamageGameplayEffectClass,
	TSubclassOf<UGameplayEffect> InHitstunGameplayEffectClass,
	float InGravityScale,
	float InMaxFlightTime)
{
	UWorld* World = GetWorld();
	if (!HasAuthority() || !World)
	{
		return;
	}

	CarriedItem = WeaponItem;
	DamageParams = InDamageParams;
	DamageGameplayEffectClass = InDamageGameplayEffectClass;
	HitstunGameplayEffectClass = InHitstunGameplayEffectClass;

	if (CarriedItem)
	{
		CarriedItem->SetWorldPhysicsEnabled(false);
		CarriedItem->SetPickupEnabled(false);
		CarriedItem->AttachToComponent(
			ThrowCollision,
			FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		CarriedItem->ForceNetUpdate();
	}

	if (AActor* ThrowingActor = GetInstigator())
	{
		ThrowCollision->IgnoreActorWhenMoving(ThrowingActor, true);
	}

	ProjectileMovement->ProjectileGravityScale = InGravityScale;
	ProjectileMovement->Velocity = LaunchVelocity;
	ProjectileMovement->Activate();

	if (InMaxFlightTime > 0.0f)
	{
		World->GetTimerManager().SetTimer(
			FlightTimerHandle,
			this,
			&ThisClass::OnFlightTimeExpired,
			InMaxFlightTime,
			false);
	}
}

void AGProjectThrownWeapon::OnThrowCollisionHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	if (!HasAuthority() || bHasImpacted)
	{
		return;
	}

	if (OtherActor == this || OtherActor == CarriedItem || OtherActor == GetInstigator())
	{
		return;
	}

	bHasImpacted = true;
	ProjectileMovement->StopMovementImmediately();
	ProjectileMovement->Deactivate();

	if (OtherActor && CarriedItem && CarriedItem->ShouldApplyThrowImpactDamage())
	{
		ApplyThrowHit(OtherActor);
	}

	ReleaseCarriedItem(Hit.ImpactPoint);
	Destroy();
}

void AGProjectThrownWeapon::OnFlightTimeExpired()
{
	if (!HasAuthority() || bHasImpacted)
	{
		return;
	}

	bHasImpacted = true;
	ProjectileMovement->StopMovementImmediately();
	ProjectileMovement->Deactivate();

	ReleaseCarriedItem(GetActorLocation());
	Destroy();
}

void AGProjectThrownWeapon::ApplyThrowHit(AActor* Target)
{
	AActor* ThrowingActor = GetInstigator();
	UAbilitySystemComponent* TargetASC =
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
	if (!ThrowingActor || !TargetASC || !DamageParams.SourceAbilitySystemComponent)
	{
		return;
	}

	FGProjectDamageEffectParams HitDamageParams = DamageParams;
	HitDamageParams.TargetAbilitySystemComponent = TargetASC;
	UGProjectAbilitySystemLibrary::SetKnockbackDirection(
		HitDamageParams,
		Target->GetActorLocation() - ThrowingActor->GetActorLocation()
	);

	const FGameplayEffectContextHandle DamageContext = UGProjectAbilitySystemLibrary::ApplyDamageEffect(
		HitDamageParams,
		DamageGameplayEffectClass);
	if (!DamageContext.IsValid())
	{
		return;
	}

	if (HitDamageParams.CausesKnockdown())
	{
		UGProjectAbilitySystemLibrary::SendKnockdownEvent(HitDamageParams);
	}
	else
	{
		UGProjectAbilitySystemLibrary::ApplyHitstunEffect(HitDamageParams, HitstunGameplayEffectClass);
		UGProjectAbilitySystemLibrary::SendHitReactEvent(HitDamageParams);
	}

	if (ACharacter* TargetCharacter = Cast<ACharacter>(Target);
		TargetCharacter && !HitDamageParams.KnockbackForce.IsNearlyZero())
	{
		FVector LaunchVelocity = HitDamageParams.KnockbackForce;
		LaunchVelocity.Z = 200.0f;
		TargetCharacter->LaunchCharacter(LaunchVelocity, true, true);
	}
}

void AGProjectThrownWeapon::ReleaseCarriedItem(const FVector& ImpactLocation)
{
	if (!HasAuthority() || !CarriedItem)
	{
		return;
	}

	AGProjectItemActorBase* DroppedItem = CarriedItem;
	CarriedItem = nullptr;

	FRotator DropRotation = LandedRotation;
	DropRotation.Yaw += DroppedItem->GetActorRotation().Yaw;
	const FVector DropLocation = FindGroundLocation(ImpactLocation);

	DroppedItem->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	DroppedItem->SetReplicateMovement(true);
	DroppedItem->SetWorldPhysicsEnabled(false);
	DroppedItem->SetActorLocationAndRotation(
		DropLocation,
		DropRotation,
		false,
		nullptr,
		ETeleportType::TeleportPhysics
	);
	DroppedItem->SetWorldPhysicsEnabled(true);
	DroppedItem->SetPickupEnabled(true);
	DroppedItem->SetActorHiddenInGame(false);
	DroppedItem->OnThrowLanded();
	DroppedItem->ForceNetUpdate();
}

void AGProjectThrownWeapon::AttachCarriedItemVisual()
{
	if (!CarriedItem)
	{
		return;
	}

	CarriedItem->SetActorHiddenInGame(false);
	CarriedItem->AttachToComponent(
		ThrowCollision,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale);
}

FVector AGProjectThrownWeapon::FindGroundLocation(const FVector& Origin) const
{
	const UWorld* World = GetWorld();
	if (!World || GroundTraceDistance <= 0.0f)
	{
		return Origin;
	}

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ThrownWeaponGroundTrace), false, this);
	QueryParams.AddIgnoredActor(GetInstigator());
	QueryParams.AddIgnoredActor(CarriedItem);

	FHitResult GroundHit;
	const FVector TraceEnd = Origin - FVector(0.0f, 0.0f, GroundTraceDistance);
	if (World->LineTraceSingleByChannel(GroundHit, Origin, TraceEnd, ECC_WorldStatic, QueryParams))
	{
		return GroundHit.ImpactPoint + FVector(0.0f, 0.0f, GroundClearance);
	}

	return Origin;
}
