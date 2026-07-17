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
class UAnimMontage;

UCLASS(Abstract)
class PROJECTG_API AGProjectItemActorBase : public AActor
{
	GENERATED_BODY()

public:
	AGProjectItemActorBase();

	virtual void OnConstruction(const FTransform& Transform) override;

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
	virtual UAnimMontage* GetUseMontage() const { return nullptr; }

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Item")
	bool Use(AGProjectCharacter* Character);
	virtual bool Use_Implementation(AGProjectCharacter* Character);

	void SetPickupEnabled(bool bEnabled);
	void SetWorldPhysicsEnabled(bool bEnabled);

	void ResetToSpawnTransform();

protected:
	virtual void BeginPlay() override;

	void ApplyItemMesh();

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

private:
	FTransform InitialSpawnTransform = FTransform::Identity;
};
