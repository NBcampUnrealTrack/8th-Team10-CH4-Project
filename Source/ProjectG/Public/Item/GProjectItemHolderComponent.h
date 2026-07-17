// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GProjectItemHolderComponent.generated.h"

class AGProjectCharacter;
class AGProjectItemActorBase;

UCLASS(ClassGroup = (Item), meta = (BlueprintSpawnableComponent))
class PROJECTG_API UGProjectItemHolderComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGProjectItemHolderComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "Item")
	void TryPickupNearby();

	UFUNCTION(BlueprintCallable, Category = "Item")
	void PickupItem(AGProjectItemActorBase* Item);

	UFUNCTION(BlueprintCallable, Category = "Item")
	void UseHeldItem();

	UFUNCTION(BlueprintCallable, Category = "Item")
	void DropHeldItem();

	AGProjectItemActorBase* ReleaseHeldItem();

	UFUNCTION(BlueprintPure, Category = "Item")
	bool HasHeldItem() const { return HeldItem != nullptr; }

	UFUNCTION(BlueprintPure, Category = "Item")
	AGProjectItemActorBase* GetHeldItem() const { return HeldItem; }

	bool HasNearbyPickup();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Settings")
	FName WeaponSocketName = TEXT("WeaponSocket");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Settings", meta = (ClampMin = "0.0"))
	float PickupSearchRadius = 150.0f;

private:
	UFUNCTION()
	void OnRep_HeldItem();

	UFUNCTION(Server, Reliable)
	void ServerTryPickupNearby();

	UFUNCTION(Server, Reliable)
	void ServerPickupItem(AGProjectItemActorBase* Item);

	UFUNCTION(Server, Reliable)
	void ServerUseHeldItem();

	UFUNCTION(Server, Reliable)
	void ServerDropHeldItem();

	AGProjectCharacter* GetOwnerCharacter() const;
	AGProjectItemActorBase* FindClosestPickup() const;
	void TryPickupNearbyInternal();
	void PickupItemInternal(AGProjectItemActorBase* Item);
	void UseHeldItemInternal();
	void DropHeldItemInternal();
	void DropItemToGround(AGProjectItemActorBase* ItemToDrop, AGProjectItemActorBase* IgnoreIncomingItem, const FVector& DropOrigin);
	FVector GetDropLocationGround(const FVector& StartOrigin, AActor* IgnoreActor1 = nullptr, AActor* IgnoreActor2 = nullptr) const;
	FVector GetDropLocationForItem(AGProjectItemActorBase* ItemToDrop, AGProjectItemActorBase* IgnoreIncomingItem, const FVector& DropOrigin) const;
	void ApplyHeldItemAttachment();
	void ClearLocalAttachment();
	FName GetHoldSocketName(const AGProjectItemActorBase* Item) const;

	UPROPERTY(ReplicatedUsing = OnRep_HeldItem, VisibleInstanceOnly, BlueprintReadOnly, Category = "Item", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AGProjectItemActorBase> HeldItem;

	UPROPERTY(Transient)
	TObjectPtr<AGProjectItemActorBase> LocallyAttachedItem;
};
