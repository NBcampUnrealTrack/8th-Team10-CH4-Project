// Copyright Epic Games, Inc. All Rights Reserved.

#include "Item/GProjectItemHolderComponent.h"

#include "Character/GProjectCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/OverlapResult.h"
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
	DOREPLIFETIME(UGProjectItemHolderComponent, bHeldItemReleasedForThrow);
}

void UGProjectItemHolderComponent::OnRep_HeldItem()
{
	if (!HeldItem && bHeldItemReleasedForThrow)
	{
		ClearLocalAttachment();
		return;
	}

	ApplyHeldItemAttachment();
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

AGProjectItemActorBase* UGProjectItemHolderComponent::ReleaseHeldItem()
{
	AGProjectCharacter* Character = GetOwnerCharacter();
	if (!Character || !Character->HasAuthority() || !HeldItem)
	{
		return nullptr;
	}

	AGProjectItemActorBase* ReleasedItem = HeldItem;
	HeldItem = nullptr;
	bHeldItemReleasedForThrow = true;
	LocallyAttachedItem = nullptr;

	ReleasedItem->HandleUnequipped(Character);
	ReleasedItem->ForceNetUpdate();
	Character->ForceNetUpdate();

	return ReleasedItem;
}

bool UGProjectItemHolderComponent::HasNearbyPickup()
{
	return FindClosestPickup() != nullptr;
}

void UGProjectItemHolderComponent::ServerTryPickupNearby_Implementation()
{
	TryPickupNearbyInternal();
}

void UGProjectItemHolderComponent::TryPickupNearbyInternal()
{
	PickupItemInternal(FindClosestPickup());
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
	if (!Character || !IsValid(HeldItem) || !HeldItem->Use(Character))
	{
		return;
	}
	AGProjectItemActorBase* UsedItem = HeldItem;
	HeldItem = nullptr;
	bHeldItemReleasedForThrow = false;
	LocallyAttachedItem = nullptr;

	if (IsValid(UsedItem))
	{
		UsedItem->HandleUnequipped(Character);
		if (UsedItem->ShouldDestroyOnUse())
		{
			UsedItem->Destroy();
		}
		else
		{
			UsedItem->SetPickupEnabled(false);
			UsedItem->ForceNetUpdate();
		}
	}
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

AGProjectItemActorBase* UGProjectItemHolderComponent::FindClosestPickup() const
{
	AGProjectCharacter* Character = GetOwnerCharacter();
	UWorld* World = GetWorld();
	if (!Character || !World)
	{
		return nullptr;
	}

	AGProjectItemActorBase* ClosestItem = nullptr;
	float ClosestDistanceSquared = TNumericLimits<float>::Max();

	TArray<FOverlapResult> OverlapResults;
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_PhysicsBody);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(PickupItemOverlap), false, Character);
	const bool bHasOverlap = World->OverlapMultiByObjectType(
		OverlapResults,
		Character->GetActorLocation(),
		FQuat::Identity,
		ObjectQueryParams,
		FCollisionShape::MakeSphere(PickupSearchRadius),
		QueryParams);
	if (!bHasOverlap)
	{
		return nullptr;
	}

	TSet<AGProjectItemActorBase*> CheckedItems;
	for (const FOverlapResult& OverlapResult : OverlapResults)
	{
		AGProjectItemActorBase* Item = Cast<AGProjectItemActorBase>(OverlapResult.GetActor());
		if (!Item || CheckedItems.Contains(Item))
		{
			continue;
		}
		CheckedItems.Add(Item);

		if (!Item->CanBePickedUpBy(Character))
		{
			continue;
		}

		const float DistanceSquared = FVector::DistSquared2D(
			Character->GetActorLocation(),
			Item->GetPickupLocation());
		if (DistanceSquared < ClosestDistanceSquared)
		{
			ClosestDistanceSquared = DistanceSquared;
			ClosestItem = Item;
		}
	}

	return ClosestItem;
}

void UGProjectItemHolderComponent::PickupItemInternal(AGProjectItemActorBase* Item)
{
	AGProjectCharacter* Character = GetOwnerCharacter();
	if (!Character || !Character->HasAuthority() || !Item || !Item->CanBePickedUpBy(Character))
	{
		return;
	}

	if (HeldItem == Item)
	{
		return;
	}

	if (HeldItem)
	{
		DropHeldItemInternal();
	}

	HeldItem = Item;
	bHeldItemReleasedForThrow = false;
	ApplyHeldItemAttachment();
	HeldItem->ForceNetUpdate();
	Character->ForceNetUpdate();
}

void UGProjectItemHolderComponent::ApplyHeldItemAttachment()
{
	AGProjectCharacter* Character = GetOwnerCharacter();
	if (!Character)
	{
		return;
	}

	if (LocallyAttachedItem && LocallyAttachedItem != HeldItem)
	{
		if (Character->HasAuthority())
		{
			LocallyAttachedItem->HandleUnequipped(Character);
		}
		else
		{
			ClearLocalAttachment();
		}
	}

	if (HeldItem)
	{
		HeldItem->HandleEquipped(Character, GetHoldSocketName(HeldItem));
	}

	LocallyAttachedItem = HeldItem;
}

void UGProjectItemHolderComponent::ClearLocalAttachment()
{
	if (!LocallyAttachedItem)
	{
		return;
	}

	LocallyAttachedItem->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	LocallyAttachedItem = nullptr;
}

FName UGProjectItemHolderComponent::GetHoldSocketName(const AGProjectItemActorBase* Item) const
{
	return Item && Item->UsesWeaponSocket() ? WeaponSocketName : ItemSocketName;
}

void UGProjectItemHolderComponent::DropHeldItemInternal()
{
	AGProjectCharacter* Character = GetOwnerCharacter();
	UWorld* World = GetWorld();
	if (!Character || !HeldItem || !World)
	{
		return;
	}

	AGProjectItemActorBase* DroppedItem = HeldItem;
	HeldItem = nullptr;
	bHeldItemReleasedForThrow = false;
	LocallyAttachedItem = nullptr;
	DroppedItem->HandleUnequipped(Character);

	const FVector DropOrigin = Character->GetActorLocation() +
		Character->GetActorForwardVector() * 120.0f +
		FVector(0.0f, 0.0f, 40.0f);

	FVector DropLocation = DropOrigin;

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(DropItemGroundTrace), false, Character);
	QueryParams.AddIgnoredActor(DroppedItem);

	FHitResult GroundHit;
	const FVector TraceEnd = DropOrigin - FVector(0.0f, 0.0f, 500.0f);
	if (World->LineTraceSingleByChannel(GroundHit, DropOrigin, TraceEnd, ECC_WorldStatic, QueryParams))
	{
		DropLocation = GroundHit.ImpactPoint + FVector(0.0f, 0.0f, 20.0f);
	}

	DroppedItem->SetActorLocation(
		DropLocation,
		false,
		nullptr,
		ETeleportType::TeleportPhysics);
	DroppedItem->SetPickupEnabled(true);

	if (UStaticMeshComponent* ItemMesh = DroppedItem->GetItemMesh())
	{
		ItemMesh->AddImpulse(
			Character->GetActorForwardVector() * 300.0f + FVector(0.0f, 0.0f, 150.0f),
			NAME_None,
			true);
	}

	DroppedItem->ForceNetUpdate();
	Character->ForceNetUpdate();
}
