// Copyright Epic Games, Inc. All Rights Reserved.

#include "Item/GProjectItemHolderComponent.h"

#include "Character/GProjectCharacter.h"
#include "Item/GProjectItemActorBase.h"
#include "Net/UnrealNetwork.h"

UGProjectItemHolderComponent::UGProjectItemHolderComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UGProjectItemHolderComponent::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UGProjectItemHolderComponent, HeldItem);
}

void UGProjectItemHolderComponent::TryPickupNearby()
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		TryPickupNearbyInternal();
		return;
	}

	ServerTryPickupNearby();
}

void UGProjectItemHolderComponent::PickupItem(AGProjectItemActorBase* Item)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		PickupItemInternal(Item);
		return;
	}

	ServerPickupItem(Item);
}

void UGProjectItemHolderComponent::UseHeldItem()
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		UseHeldItemInternal();
		return;
	}

	ServerUseHeldItem();
}

void UGProjectItemHolderComponent::DropHeldItem()
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		DropHeldItemInternal();
		return;
	}

	ServerDropHeldItem();
}

void UGProjectItemHolderComponent::ServerTryPickupNearby_Implementation()
{
	TryPickupNearbyInternal();
}

void UGProjectItemHolderComponent::TryPickupNearbyInternal()
{
	AGProjectCharacter* Character = GetOwnerCharacter();
	if (!Character)
	{
		return;
	}

	TArray<AActor*> OverlappingItems;
	Character->GetOverlappingActors(OverlappingItems, AGProjectItemActorBase::StaticClass());

	AGProjectItemActorBase* ClosestItem = nullptr;
	float ClosestDistanceSquared = TNumericLimits<float>::Max();
	for (AActor* OverlappingActor : OverlappingItems)
	{
		AGProjectItemActorBase* Item = Cast<AGProjectItemActorBase>(OverlappingActor);
		if (!Item || !Item->CanBePickedUpBy(Character))
		{
			continue;
		}

		const float DistanceSquared = FVector::DistSquared(
			Character->GetActorLocation(), Item->GetActorLocation());
		if (DistanceSquared < ClosestDistanceSquared)
		{
			ClosestDistanceSquared = DistanceSquared;
			ClosestItem = Item;
		}
	}
	PickupItemInternal(ClosestItem);
}

void UGProjectItemHolderComponent::ServerPickupItem_Implementation(AGProjectItemActorBase* Item)
{
	PickupItemInternal(Item);
}

void UGProjectItemHolderComponent::ServerUseHeldItem_Implementation()
{
	UseHeldItemInternal();
}

void UGProjectItemHolderComponent::UseHeldItemInternal()
{
	AGProjectCharacter* Character = GetOwnerCharacter();
	if (!Character || !HeldItem || !HeldItem->Use(Character))
	{
		return;
	}

	AGProjectItemActorBase* UsedItem = HeldItem;
	HeldItem = nullptr;
	UsedItem->HandleUnequipped(Character);
	UsedItem->Destroy();
	Character->ForceNetUpdate();
}

void UGProjectItemHolderComponent::ServerDropHeldItem_Implementation()
{
	DropHeldItemInternal();
}

AGProjectCharacter* UGProjectItemHolderComponent::GetOwnerCharacter() const
{
	return Cast<AGProjectCharacter>(GetOwner());
}

void UGProjectItemHolderComponent::PickupItemInternal(AGProjectItemActorBase* Item)
{
	AGProjectCharacter* Character = GetOwnerCharacter();
	if (!Character || !Item || !Item->CanBePickedUpBy(Character))
	{
		return;
	}

	if (HeldItem == Item)
	{
		return;
	}

	DropHeldItemInternal();
	HeldItem = Item;
	HeldItem->HandleEquipped(Character);
	HeldItem->ForceNetUpdate();
	Character->ForceNetUpdate();
}

void UGProjectItemHolderComponent::DropHeldItemInternal()
{
	AGProjectCharacter* Character = GetOwnerCharacter();
	if (!Character || !HeldItem)
	{
		return;
	}

	AGProjectItemActorBase* DroppedItem = HeldItem;
	HeldItem = nullptr;
	DroppedItem->HandleUnequipped(Character);
	DroppedItem->SetActorLocation(
		Character->GetActorLocation() + Character->GetActorForwardVector() * 120.0f + FVector(0.0f, 0.0f, 40.0f));
	DroppedItem->SetPickupEnabled(true);
	DroppedItem->ForceNetUpdate();
	Character->ForceNetUpdate();
}
