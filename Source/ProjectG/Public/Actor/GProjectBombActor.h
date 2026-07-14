// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/GProjectAbilitySystemLibrary.h"
#include "GameFramework/Actor.h"
#include "GProjectBombActor.generated.h"

class UDecalComponent;
class UGameplayEffect;
class UAbilitySystemComponent;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class UParticleSystem;
class USceneComponent;
class USoundBase;
class UStaticMeshComponent;

UCLASS()
class PROJECTG_API AGProjectBombActor : public AActor
{
	GENERATED_BODY()

public:
	AGProjectBombActor();

	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "Bomb")
	void SetSourceActor(AActor* InSourceActor);

	UFUNCTION(BlueprintCallable, Category = "Bomb")
	void StartFuse();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bomb")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bomb")
	TObjectPtr<UStaticMeshComponent> BombMesh;

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bomb|Warning", meta = (ClampMin = "1.0"))
	float DecalProjectionDepth = 256.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bomb|Blink")
	FName BombFlashParameterName = TEXT("BombFlashAmount");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bomb|Blink", meta = (ClampMin = "0.01"))
	float MaxBlinkInterval = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bomb|Blink", meta = (ClampMin = "0.01"))
	float MinBlinkInterval = 0.08f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bomb|Blink", meta = (ClampMin = "0.0"))
	float BombFlashAmount = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bomb|Feedback")
	TObjectPtr<USoundBase> ExplosionSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bomb|Feedback")
	TObjectPtr<UParticleSystem> ExplosionEffect;

private:
	UFUNCTION()
	void OnRep_FuseStarted();

	void StartFuseVisuals();
	void UpdateWarningDecal();
	void UpdateBombBlink(float DeltaSeconds);
	void Explode();
	void ApplyExplosionToTarget(AActor* TargetActor);

	UAbilitySystemComponent* GetSourceAbilitySystemComponent() const;

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayExplosionFeedback();

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> BombMaterial;

	UPROPERTY()
	TObjectPtr<AActor> SourceActor;

	float ElapsedTime = 0.0f;
	float BlinkElapsedTime = 0.0f;
	UPROPERTY(ReplicatedUsing = OnRep_FuseStarted)
	bool bFuseStarted = false;
	bool bBlinkOn = false;
	bool bExploded = false;
};
