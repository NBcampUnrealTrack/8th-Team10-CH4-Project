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

	UFUNCTION(BlueprintPure, Category = "Item")
	bool HasHeldItem() const { return HeldItem != nullptr; }

	UFUNCTION(BlueprintPure, Category = "Item")
	AGProjectItemActorBase* GetHeldItem() const { return HeldItem; }

private:
	UFUNCTION(Server, Reliable)
	void ServerTryPickupNearby();

	UFUNCTION(Server, Reliable)
	void ServerPickupItem(AGProjectItemActorBase* Item);

	UFUNCTION(Server, Reliable)
	void ServerUseHeldItem();

	UFUNCTION(Server, Reliable)
	void ServerDropHeldItem();

	AGProjectCharacter* GetOwnerCharacter() const;
	void TryPickupNearbyInternal();
	void PickupItemInternal(AGProjectItemActorBase* Item);
	void UseHeldItemInternal();
	void DropHeldItemInternal();

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Item", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AGProjectItemActorBase> HeldItem;
};
