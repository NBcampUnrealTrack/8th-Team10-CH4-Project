// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GProjectItemActorBase.generated.h"

class AGProjectCharacter;
class UGProjectItemDefinition;
class USphereComponent;
class UStaticMeshComponent;

UCLASS(Abstract)
class PROJECTG_API AGProjectItemActorBase : public AActor
{
	GENERATED_BODY()

public:
	AGProjectItemActorBase();

	virtual void OnConstruction(const FTransform& Transform) override;

	UGProjectItemDefinition* GetItemDefinition() const { return ItemDefinition; }
	UStaticMeshComponent* GetItemMesh() const { return ItemMesh; }

	virtual bool CanBePickedUpBy(const AGProjectCharacter* Character) const;
	virtual void HandleEquipped(AGProjectCharacter* Character);
	virtual void HandleUnequipped(AGProjectCharacter* Character);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Item")
	bool Use(AGProjectCharacter* Character);
	virtual bool Use_Implementation(AGProjectCharacter* Character);

	void SetPickupEnabled(bool bEnabled);

	void ResetToSpawnTransform();

protected:
	virtual void BeginPlay() override;

	void ApplyDefinitionMesh();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
	TObjectPtr<USphereComponent> PickupCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
	TObjectPtr<UStaticMeshComponent> ItemMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	TObjectPtr<UGProjectItemDefinition> ItemDefinition;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (ClampMin = "0.0"))
	float MaxPickupDistance = 250.0f;

private:
	FTransform InitialSpawnTransform = FTransform::Identity;
};
