// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/GProjectAbilitySystemLibrary.h"
#include "Item/GProjectItemActorBase.h"
#include "GProjectBombActor.generated.h"

class AGProjectCharacter;
class UDecalComponent;
class UGameplayEffect;
class UAbilitySystemComponent;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class UParticleSystem;
class USoundBase;

UCLASS()
class PROJECTG_API AGProjectBombActor : public AGProjectItemActorBase
{
	GENERATED_BODY()

public:
	AGProjectBombActor();

	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ShouldDestroyOnUse() const override;
	virtual bool ShouldApplyThrowImpactDamage() const override;
	virtual void OnThrowStarted(AGProjectCharacter* Thrower) override;
	virtual bool Use_Implementation(AGProjectCharacter* Character) override;

	UFUNCTION(BlueprintCallable, Category = "Bomb")
	void SetSourceActor(AActor* InSourceActor);

	UFUNCTION(BlueprintCallable, Category = "Bomb")
	void StartFuse();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bomb|Warning")
	TObjectPtr<UDecalComponent> WarningDecal;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bomb|Timing", meta = (ClampMin = "0.01"))
	float FuseTime = 3.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bomb|Explosion", meta = (ClampMin = "0.0"))
	float ExplosionRadius = 350.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bomb|Explosion")
	FGProjectDamageEffectParams ExplosionDamageParams;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bomb|Explosion")
	TSubclassOf<UGameplayEffect> DamageGameplayEffectClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bomb|Explosion")
	TSubclassOf<UGameplayEffect> HitstunGameplayEffectClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bomb|Warning")
	TObjectPtr<UMaterialInterface> WarningDecalMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bomb|Feedback")
	TObjectPtr<USoundBase> ExplosionSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bomb|Feedback")
	TObjectPtr<UParticleSystem> ExplosionEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bomb|Feedback")
	FVector ExplosionEffectScale = FVector::OneVector;

private:
	UFUNCTION()
	void OnRep_FuseStarted();

	void StartFuseVisuals();
	void UpdateWarningDecal();
	void UpdateBombBlink();
	void Explode();
	void ApplyExplosionToTarget(AActor* TargetActor);

	UAbilitySystemComponent* GetSourceAbilitySystemComponent() const;

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayExplosionFeedback();

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> BombMaterial;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> WarningDecalMaterialInstance;

	UPROPERTY()
	TObjectPtr<AActor> SourceActor;

	float ElapsedTime = 0.0f;
	UPROPERTY(ReplicatedUsing = OnRep_FuseStarted)
	bool bFuseStarted = false;
	bool bExploded = false;
};
