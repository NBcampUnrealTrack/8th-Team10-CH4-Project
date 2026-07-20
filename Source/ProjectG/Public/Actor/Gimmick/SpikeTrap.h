// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TimelineComponent.h"
#include "AbilitySystemInterface.h"
#include "SpikeTrap.generated.h"

class UCurveFloat;
class UBoxComponent;
class UAbilitySystemComponent;
class UGameplayEffect;
class USoundBase;
class ADestroyWall;

UCLASS()
class PROJECTG_API ASpikeTrap : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ASpikeTrap();

	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return nullptr; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* SceneRootComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* BaseMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* SpikeMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* TriggerBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UAbilitySystemComponent* AbilitySystemComponent;

	// 밟은 후 가시가 올라오기 전까지의 예고 시간(초)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Setting", meta = (ClampMin = "0.0"))
	float WarningDelay = 0.4f;

	// 가시가 올라오는 높이(cm)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Setting", meta = (ClampMin = "0.0"))
	float RiseHeight = 100.0f;

	// 다 올라온 상태로 유지되는 시간(초)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Setting", meta = (ClampMin = "0.0"))
	float ExtendedHoldTime = 0.5f;

	// 다시 트리거 가능해지기까지의 대기 시간(초)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Setting", meta = (ClampMin = "0.0"))
	float CooldownTime = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Damage")
	float DamageAmount = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Damage")
	TSubclassOf<UGameplayEffect> SpikeDamageEffectClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gimmick|Sound")
	USoundBase* SpikeSound;

	// 트리거 시점 대비 사운드 재생을 미루는 시간(초). 가시가 눈에 보이는 타이밍에 맞추는 용도
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Sound", meta = (ClampMin = "0.0"))
	float SpikeSoundDelay = 0.6f;

	// 스파이크 사운드 음량 배율
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Sound", meta = (ClampMin = "0.0"))
	float SpikeSoundVolume = 1.0f;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Timeline", meta = (AllowPrivateAccess = "true"))
	UCurveFloat* SpikeCurve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Timeline", meta = (AllowPrivateAccess = "true"))
	UCurveFloat* FloorSinkCurve;

	FTimeline SpikeTimeline;
	FVector FloorSinkStartLocation;

	TWeakObjectPtr<ADestroyWall> UnderlyingDestroyWall;
	FVector UnderlyingWallInitialLocation;

	UPROPERTY(ReplicatedUsing = OnRep_SpikeExtended)
	bool bSpikeExtended = false;

	bool bOnCooldown = false;
	FVector SpikeRetractedLocation;
	FVector SpikeExtendedLocation;

	FTimerHandle WarningTimerHandle;
	FTimerHandle RetractTimerHandle;
	FTimerHandle CooldownTimerHandle;
	FTimerHandle SoundTimerHandle;

	UFUNCTION()
	void OnTriggerBoxBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	void ServerExtendSpike();
	void ServerRetractSpike();

	UFUNCTION()
	void OnRep_SpikeExtended();

	void PlayExtendTimeline();
	void PlayRetractTimeline();

	UFUNCTION()
	void HandleTimelineProgress(float Value);

	UFUNCTION()
	void HandleTimelineFinished();

	void ApplyDamageToOverlappingTargets();
	void ClearCooldown();

	// 지연된 스파이크 사운드 재생
	void PlaySpikeSound();

	void HandleFloorSinkFinished();
};
