// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GProjectItemActorBase.generated.h"

class AGProjectCharacter;
class UGProjectItemDefinition;
class USphereComponent;
class UStaticMesh;
class UStaticMeshComponent;
class UTexture2D;

UCLASS(Abstract)
class PROJECTG_API AGProjectItemActorBase : public AActor
{
	GENERATED_BODY()

public:
	AGProjectItemActorBase();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Tick(float DeltaSeconds) override;

	UGProjectItemDefinition* GetItemDefinition() const { return ItemDefinition; }
	UStaticMeshComponent* GetItemMesh() const { return ItemMesh; }
	FVector GetPickupLocation() const;

	virtual bool CanBePickedUpBy(const AGProjectCharacter* Character) const;
	virtual bool CanUse(const AGProjectCharacter* Character) const;
	virtual bool UsesWeaponSocket() const;
	virtual bool ShouldDestroyOnUse() const;
	virtual bool CanBeThrown() const;
	virtual bool ShouldApplyThrowImpactDamage() const;
	virtual void OnThrowStarted(AGProjectCharacter* Thrower);
	virtual void OnThrowLanded();
	virtual void HandleEquipped(AGProjectCharacter* Character, FName HoldSocketName);
	virtual void HandleUnequipped(AGProjectCharacter* Character);
	virtual void DetachFromHolder(AGProjectCharacter* Character);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Item")
	bool Use(AGProjectCharacter* Character);
	virtual bool Use_Implementation(AGProjectCharacter* Character);

	void SetPickupEnabled(bool bEnabled);
	void BlockPickupFor(float Duration);
	void BlockPickupBriefly();
	void SetWorldPhysicsEnabled(bool bEnabled);
	void AttachItemVisual(AGProjectCharacter* Character, FName HoldSocketName);
	void DetachItemVisual();

	void ResetToSpawnTransform();

protected:
	virtual void BeginPlay() override;

	void ApplyItemMesh();
	void ApplyServerWorldPhysics(bool bEnabled);
	void ApplyClientWorldVisualState(bool bWorldState);
	void FinalizeWorldPhysicsTransform();

	UFUNCTION()
	void OnRep_WorldPhysicsEnabled();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
	TObjectPtr<USphereComponent> PickupCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
	TObjectPtr<UStaticMeshComponent> ItemMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (MultiLine = true))
	FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	TSoftObjectPtr<UStaticMesh> PickupMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Definition")
	TObjectPtr<UGProjectItemDefinition> ItemDefinition;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (ClampMin = "0.0"))
	float MaxPickupDistance = 250.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (ClampMin = "0.0"))
	float DefaultPickupBlockDuration = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Physics", meta = (ClampMin = "0.0"))
	float PhysicsSleepLinearSpeed = 3.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Physics", meta = (ClampMin = "0.0"))
	float PhysicsSleepAngularSpeed = 3.0f;

private:
	FTransform InitialSpawnTransform = FTransform::Identity;
	float PickupBlockedUntilTime = 0.0f;

	UPROPERTY(ReplicatedUsing = OnRep_WorldPhysicsEnabled)
	bool bWorldPhysicsEnabled = false;
};
