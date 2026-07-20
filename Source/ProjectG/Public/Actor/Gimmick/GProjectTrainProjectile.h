// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystem/GProjectAbilitySystemLibrary.h"
#include "AbilitySystemInterface.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GProjectTrainProjectile.generated.h"

class UAbilitySystemComponent;
class UAudioComponent;
class UBoxComponent;
class UGameplayEffect;
class UProjectileMovementComponent;
class UPrimitiveComponent;
class USkeletalMeshComponent;
class USoundBase;

UCLASS()
class PROJECTG_API AGProjectTrainProjectile : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AGProjectTrainProjectile();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	void InitTrain(
		const FVector& LaunchVelocity,
		const FGProjectDamageEffectParams& InDamageParams,
		TSubclassOf<UGameplayEffect> InDamageGameplayEffectClass,
		float InLifeTime);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Train")
	TObjectPtr<UBoxComponent> TrainCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Train")
	TObjectPtr<USkeletalMeshComponent> TrainMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Train")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Train")
	TObjectPtr<UAudioComponent> LoopAudioComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Train")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Train|Collision")
	FVector CollisionBoxExtent = FVector(250.0f, 100.0f, 100.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Train|Feedback")
	TObjectPtr<USoundBase> LoopSound;

private:
	UFUNCTION()
	void OnTrainBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	void ApplyTrainHit(AActor* Target);
	bool CanHitTarget(AActor* Target) const;

	UPROPERTY()
	FGProjectDamageEffectParams DamageParams;

	UPROPERTY()
	TSubclassOf<UGameplayEffect> DamageGameplayEffectClass;
};
