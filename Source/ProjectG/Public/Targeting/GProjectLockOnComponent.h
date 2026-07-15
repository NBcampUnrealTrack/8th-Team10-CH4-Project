// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GProjectLockOnComponent.generated.h"

class AActor;
class UWidgetComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGProjectLockOnTargetChangedSignature, AActor*, NewTarget);

UCLASS(ClassGroup = (Targeting), meta = (BlueprintSpawnableComponent))
class PROJECTG_API UGProjectLockOnComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGProjectLockOnComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "Lock On")
	bool StartLockOn();

	UFUNCTION(BlueprintCallable, Category = "Lock On")
	void ClearLockOn();

	UFUNCTION(BlueprintPure, Category = "Lock On")
	AActor* GetCurrentTarget() const { return CurrentTarget; }

	UPROPERTY(BlueprintAssignable, Category = "Lock On")
	FGProjectLockOnTargetChangedSignature OnLockOnTargetChanged;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lock On", meta = (ClampMin = "0.0"))
	float LockOnRadius = 1200.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lock On", meta = (ClampMin = "0.0"))
	float BreakDistance = 1500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lock On", meta = (ClampMin = "0.0"))
	float RotationInterpSpeed = 12.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lock On")
	TEnumAsByte<ECollisionChannel> VisibilityTraceChannel = ECC_Visibility;

private:
	UFUNCTION(Server, Reliable)
	void ServerSetLockOnTarget(AActor* NewTarget);

	UFUNCTION()
	void OnRep_CurrentTarget();

	AActor* FindBestTarget() const;
	bool IsValidTarget(AActor* Target) const;
	bool HasLineOfSightTo(const AActor* Target) const;
	void SetCurrentTarget(AActor* NewTarget);
	void ApplyLockOnState();
	void UpdateOwnerRotation(float DeltaTime) const;
	void UpdateLockOnWidget();

	UPROPERTY(ReplicatedUsing = OnRep_CurrentTarget, BlueprintReadOnly, Category = "Lock On", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AActor> CurrentTarget;

	bool bDefaultOrientRotationToMovement = true;

	UPROPERTY(VisibleAnywhere, Category = "Lock On")
	TObjectPtr<UWidgetComponent> LockOnWidgetComp;
	UPROPERTY(EditAnywhere, Category = "Lock On")
	TSubclassOf<UUserWidget> LockOnWidgetClass;
};
