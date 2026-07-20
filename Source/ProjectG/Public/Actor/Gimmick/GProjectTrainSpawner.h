// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystem/GProjectAbilitySystemLibrary.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GProjectTrainSpawner.generated.h"

class AGProjectTrainProjectile;
class UArrowComponent;
class UGameplayEffect;
class USceneComponent;
class USoundBase;

UCLASS()
class PROJECTG_API AGProjectTrainSpawner : public AActor
{
	GENERATED_BODY()

public:
	AGProjectTrainSpawner();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Train")
	TObjectPtr<USceneComponent> SceneRootComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Train")
	TObjectPtr<UArrowComponent> SpawnDirection;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Train|Spawn")
	TSubclassOf<AGProjectTrainProjectile> TrainClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Train|Spawn", meta = (ClampMin = "0.1"))
	float SpawnInterval = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Train|Spawn", meta = (ClampMin = "0.0"))
	float WarningLeadTime = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Train|Spawn", meta = (ClampMin = "0.0"))
	float TrainSpeed = 2500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Train|Spawn", meta = (ClampMin = "0.0"))
	float TrainLifeTime = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Train|Damage")
	FGProjectDamageEffectParams DamageParams;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Train|Damage")
	TSubclassOf<UGameplayEffect> DamageGameplayEffectClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Train|Feedback")
	TObjectPtr<USoundBase> WarningSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Train|Feedback", meta = (ClampMin = "0.0"))
	float WarningSoundVolume = 1.0f;

private:
	void StartSpawnCycle();
	void SpawnTrain();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayWarningSound();

	FTimerHandle SpawnTimerHandle;
	FTimerHandle CycleTimerHandle;
};
