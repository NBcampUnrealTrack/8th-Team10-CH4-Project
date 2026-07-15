// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystem/GProjectAbilitySystemLibrary.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GProjectThrownWeapon.generated.h"

class AGProjectItemActorBase;
class UGameplayEffect;
class UProjectileMovementComponent;
class USphereComponent;

UCLASS()
class PROJECTG_API AGProjectThrownWeapon : public AActor
{
	GENERATED_BODY()

public:
	AGProjectThrownWeapon();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void InitAndLaunch(
		AGProjectItemActorBase* WeaponItem,
		const FVector& LaunchVelocity,
		const FGProjectDamageEffectParams& InDamageParams,
		TSubclassOf<UGameplayEffect> InDamageGameplayEffectClass,
		TSubclassOf<UGameplayEffect> InHitstunGameplayEffectClass,
		float InGravityScale,
		float InMaxFlightTime);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Throw")
	TObjectPtr<USphereComponent> ThrowCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Throw")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Throw", meta = (ClampMin = "1.0"))
	float CollisionRadius = 25.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Throw", meta = (ClampMin = "0.0"))
	float GroundTraceDistance = 500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Throw")
	float GroundClearance = 20.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Throw")
	FRotator LandedRotation = FRotator(-90.0f, 0.0f, 0.0f);

private:
	UFUNCTION()
	void OnRep_CarriedItem();

	UFUNCTION()
	void OnThrowCollisionHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		FVector NormalImpulse,
		const FHitResult& Hit);

	void OnFlightTimeExpired();
	void ApplyThrowHit(AActor* Target);
	void ReleaseCarriedItem(const FVector& ImpactLocation);
	void AttachCarriedItemVisual();
	FVector FindGroundLocation(const FVector& Origin) const;

	UPROPERTY(ReplicatedUsing = OnRep_CarriedItem)
	TObjectPtr<AGProjectItemActorBase> CarriedItem;

	UPROPERTY()
	FGProjectDamageEffectParams DamageParams;

	UPROPERTY()
	TSubclassOf<UGameplayEffect> DamageGameplayEffectClass;

	UPROPERTY()
	TSubclassOf<UGameplayEffect> HitstunGameplayEffectClass;

	FTimerHandle FlightTimerHandle;
	bool bHasImpacted = false;
};
