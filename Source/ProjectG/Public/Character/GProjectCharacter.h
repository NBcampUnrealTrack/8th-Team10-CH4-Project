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
	UMeshComponent* GetAttackTraceMesh() const;
	FName GetAttackTraceStartSocketName() const;
	FName GetAttackTraceEndSocketName() const;

	UFUNCTION(BlueprintCallable, Category = "Combat|Trace")
	void SetAttackTraceSource(UMeshComponent* InTraceMesh, FName InStartSocket, FName InEndSocket);

	UFUNCTION(BlueprintCallable, Category = "Combat|Trace")
	void ResetAttackTraceSource();

	UFUNCTION(BlueprintCallable, Category = "Death")
	virtual void HandleDeath();

	UFUNCTION(BlueprintPure, Category = "Death")
	bool IsDead() const;

protected:
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
	TArray<TSubclassOf<UGProjectGameplayAbility>> StartupAbilities;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death")
	float RespawnDelay = 3.0f;

private:
	void InitAbilityActorInfo();
	void AddCharacterAbilities();

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Death", meta = (AllowPrivateAccess = "true"))
	bool bDead = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> TopDownCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Targeting", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGProjectLockOnComponent> LockOnComponent;

	UPROPERTY(Transient)
	TObjectPtr<UMeshComponent> AttackTraceMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Trace")
	FName DefaultTraceStartSocket = TEXT("hand_r");

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Trace")
	FName DefaultTraceEndSocket = TEXT("hand_l");

	FName AttackTraceStartSocket;
	FName AttackTraceEndSocket;
};
