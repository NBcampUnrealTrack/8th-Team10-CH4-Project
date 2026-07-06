// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "GProjectCharacter.generated.h"

class UAbilitySystemComponent;
class UCameraComponent;
class UMeshComponent;
class UGProjectAbilitySystemComponent;
class UGProjectGameplayAbility;
class UGProjectLockOnComponent;
class UGProjectComboData;
class UGProjectItemHolderComponent;
class UGameplayEffect;
class USpringArmComponent;

UCLASS()
class PROJECTG_API AGProjectCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AGProjectCharacter();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UGProjectAbilitySystemComponent* GetGProjectAbilitySystemComponent() const;
	UGProjectLockOnComponent* GetLockOnComponent() const;
	UGProjectItemHolderComponent* GetItemHolderComponent() const;
	UGProjectComboData* GetActiveGroundComboData() const;
	UGProjectComboData* GetActiveAirComboData() const;
	UGProjectComboData* GetActiveDashComboData() const;
	UMeshComponent* GetAttackTraceMesh() const;
	FName GetAttackTraceStartSocketName() const;
	FName GetAttackTraceEndSocketName() const;

	void ResetForNewRound(const FTransform& SpawnTransform);

	UFUNCTION(BlueprintCallable, Category = "Combat|Trace")
	void SetAttackTraceSource(UMeshComponent* InTraceMesh, FName InStartSocket, FName InEndSocket);

	UFUNCTION(BlueprintCallable, Category = "Combat|Trace")
	void ResetAttackTraceSource();

	UFUNCTION(BlueprintCallable, Category = "Combat|Combo")
	void SetActiveComboData(
		UGProjectComboData* NewGroundComboData,
		UGProjectComboData* NewAirComboData,
		UGProjectComboData* NewDashComboData);

	UFUNCTION(BlueprintCallable, Category = "Combat|Combo")
	void ResetActiveComboData();

	UFUNCTION(BlueprintCallable, Category = "Death")
	virtual void HandleDeath();

	UFUNCTION(BlueprintPure, Category = "Death")
	bool IsDead() const;

	UFUNCTION(BlueprintCallable, Category = "Movement|Sprint")
	void StartSprint();

	UFUNCTION(BlueprintCallable, Category = "Movement|Sprint")
	void StopSprint();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode = 0) override;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Settings")
	TArray<TSubclassOf<UGProjectGameplayAbility>> StartupAbilities;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Settings")
	TObjectPtr<UGProjectComboData> DefaultGroundComboData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Settings")
	TObjectPtr<UGProjectComboData> DefaultAirComboData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Settings")
	TObjectPtr<UGProjectComboData> DefaultDashComboData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death")
	float RespawnDelay = 3.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Sprint", meta = (ClampMin = "0.0"))
	float WalkSpeed = 450.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Sprint", meta = (ClampMin = "0.0"))
	float SprintSpeed = 750.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Sprint", meta = (ClampMin = "0.0"))
	float DashingSpeedThreshold = 650.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Resource|SP")
	TSubclassOf<UGameplayEffect> SPRegenGameplayEffectClass;

private:
	void InitAbilityActorInfo();
	void AddCharacterAbilities();
	void RefreshMovementStateTags();
	void SetSprintRequested(bool bRequested);
	void ApplySPRegenEffect();

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Death", meta = (AllowPrivateAccess = "true"))
	bool bDead = false;

	bool bSprintRequested = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> TopDownCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Settings", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGProjectLockOnComponent> LockOnComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item|Settings", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGProjectItemHolderComponent> ItemHolderComponent;

	UPROPERTY(Transient)
	TObjectPtr<UMeshComponent> AttackTraceMesh;

	FName AttackTraceStartSocket;
	FName AttackTraceEndSocket;

	UPROPERTY(Replicated)
	TObjectPtr<UGProjectComboData> ActiveGroundComboData;

	UPROPERTY(Replicated)
	TObjectPtr<UGProjectComboData> ActiveAirComboData;

	UPROPERTY(Replicated)
	TObjectPtr<UGProjectComboData> ActiveDashComboData;
};
