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

void UGProjectItemHolderComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UGProjectItemHolderComponent, HeldItem);
}

void UGProjectItemHolderComponent::OnRep_HeldItem()
{
    if (!HeldItem) { ClearLocalAttachment(); return; }
    ApplyHeldItemAttachment();
}

void UGProjectItemHolderComponent::TryPickupNearby()
{
    if (GetOwner() && GetOwner()->HasAuthority()) TryPickupNearbyInternal();
    else ServerTryPickupNearby();
}

void UGProjectItemHolderComponent::PickupItem(AGProjectItemActorBase* Item)
{
    if (GetOwner() && GetOwner()->HasAuthority()) PickupItemInternal(Item);
    else ServerPickupItem(Item);
}

void UGProjectItemHolderComponent::DropHeldItem()
{
    if (GetOwner() && GetOwner()->HasAuthority()) DropHeldItemInternal();
    else ServerDropHeldItem();
}

AGProjectItemActorBase* UGProjectItemHolderComponent::ReleaseHeldItem()
{
    AGProjectCharacter* Character = GetOwnerCharacter();
    if (!Character || !Character->HasAuthority() || !HeldItem) return nullptr;

    AGProjectItemActorBase* ReleasedItem = HeldItem;
    HeldItem = nullptr;
    LocallyAttachedItem = nullptr;

    ReleasedItem->DetachFromHolder(Character);
    ReleasedItem->ForceNetUpdate();
    Character->ForceNetUpdate();

    return ReleasedItem;
}

bool UGProjectItemHolderComponent::HasNearbyPickup() { return FindClosestPickup() != nullptr; }
void UGProjectItemHolderComponent::ServerTryPickupNearby_Implementation()
{
    TryPickupNearbyInternal();
}

void UGProjectItemHolderComponent::TryPickupNearbyInternal()
{
    AGProjectItemActorBase* ClosestItem = FindClosestPickup();
    PickupItemInternal(ClosestItem);
}

void UGProjectItemHolderComponent::ServerPickupItem_Implementation(AGProjectItemActorBase* Item)
{
    PickupItemInternal(Item);
}

void UGProjectItemHolderComponent::ServerDropHeldItem_Implementation()
{
    DropHeldItemInternal();
}

void UGProjectItemHolderComponent::UseHeldItem()
{
    if (GetOwner() && GetOwner()->HasAuthority()) UseHeldItemInternal();
    else ServerUseHeldItem();
}

void UGProjectItemHolderComponent::ServerUseHeldItem_Implementation() { UseHeldItemInternal(); }

void UGProjectItemHolderComponent::UseHeldItemInternal()
{
    AGProjectCharacter* Character = GetOwnerCharacter();
    if (!Character || !HeldItem || !HeldItem->Use(Character)) return;

    AGProjectItemActorBase* UsedItem = HeldItem;
    HeldItem = nullptr;
    LocallyAttachedItem = nullptr;
    UsedItem->DetachFromHolder(Character);
    
    if (UsedItem->ShouldDestroyOnUse()) UsedItem->Destroy();
    else
    {
       UsedItem->SetPickupEnabled(false);
       UsedItem->ForceNetUpdate();
    }
    Character->ForceNetUpdate();
}

AGProjectCharacter* UGProjectItemHolderComponent::GetOwnerCharacter() const
{
    return Cast<AGProjectCharacter>(GetOwner());
}

void UGProjectItemHolderComponent::ApplyHeldItemAttachment()
{
    AGProjectCharacter* Character = GetOwnerCharacter();
    if (!Character) return;

    if (LocallyAttachedItem && LocallyAttachedItem != HeldItem)
    {
        LocallyAttachedItem->DetachItemVisual();
        
        if (Character->HasAuthority())
        {
            LocallyAttachedItem->DetachFromHolder(Character);
        }
        
        LocallyAttachedItem = nullptr;
    }

    if (HeldItem)
    {
        HeldItem->HandleEquipped(Character, GetHoldSocketName(HeldItem));
    }

    LocallyAttachedItem = HeldItem;
}

void UGProjectItemHolderComponent::ClearLocalAttachment()
{
    if (!LocallyAttachedItem) return;
    LocallyAttachedItem->DetachItemVisual();
    LocallyAttachedItem = nullptr;
}

FName UGProjectItemHolderComponent::GetHoldSocketName(const AGProjectItemActorBase* Item) const
{
    static const FName ItemSocketName(TEXT("HandGrip_L"));
    return Item && Item->UsesWeaponSocket() ? WeaponSocketName : ItemSocketName;
}

void UGProjectItemHolderComponent::PickupItemInternal(AGProjectItemActorBase* Item)
{
	AGProjectCharacter* Character = GetOwnerCharacter();

	if (!Character)
	{
        return;
    }
    if (!Character->HasAuthority())
    {
        return;
    }
    if (!Item)
    {
        return;
    }
    if (!Item->CanBePickedUpBy(Character))
    {
        return;
    }
    if (HeldItem == Item)
    {
        return;
    }
    
    if (HeldItem)
    {
        AGProjectItemActorBase* PreviousItem = HeldItem;

        HeldItem = nullptr; 
        LocallyAttachedItem = nullptr;

        const FVector DropOrigin =
            Character->GetActorLocation() +
            Character->GetActorRightVector() * -90.0f +
            Character->GetActorForwardVector() * -40.0f +
            FVector(0.0f, 0.0f, 60.0f);
        PreviousItem->BlockPickupBriefly();
        DropItemToGround(PreviousItem, Item, DropOrigin);
        PreviousItem->ForceNetUpdate();
    }

    HeldItem = Item;
 
	ApplyHeldItemAttachment();
	HeldItem->ForceNetUpdate();
	Character->ForceNetUpdate();
}


FVector UGProjectItemHolderComponent::GetDropLocationGround(const FVector& StartOrigin, AActor* IgnoreActor1, AActor* IgnoreActor2) const
{
    UWorld* World = GetWorld();
    if (!World) return StartOrigin;

    FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ItemDropGroundTrace), false, GetOwner());
    if (IgnoreActor1) QueryParams.AddIgnoredActor(IgnoreActor1);
    if (IgnoreActor2) QueryParams.AddIgnoredActor(IgnoreActor2);

    FHitResult GroundHit;
    if (World->LineTraceSingleByChannel(GroundHit, StartOrigin, StartOrigin - FVector(0.0f, 0.0f, 500.0f), ECC_WorldStatic, QueryParams))
    {
       return GroundHit.ImpactPoint + FVector(0.0f, 0.0f, 20.0f);
    }
    return StartOrigin;
}

FVector UGProjectItemHolderComponent::GetDropLocationForItem(
    AGProjectItemActorBase* ItemToDrop,
    AGProjectItemActorBase* IgnoreIncomingItem,
    const FVector& DropOrigin) const
{
    FVector DropLocation = GetDropLocationGround(DropOrigin, ItemToDrop, IgnoreIncomingItem);

    UStaticMeshComponent* ItemMesh = ItemToDrop ? ItemToDrop->GetItemMesh() : nullptr;
    if (!ItemMesh)
    {
        return DropLocation;
    }

    const float HalfHeight = FMath::Max(ItemMesh->Bounds.BoxExtent.Z, 20.0f);
    DropLocation.Z += HalfHeight + 5.0f;
    return DropLocation;
}

void UGProjectItemHolderComponent::DropItemToGround(
    AGProjectItemActorBase* ItemToDrop,
    AGProjectItemActorBase* IgnoreIncomingItem,
    const FVector& DropOrigin)
{
    AGProjectCharacter* Character = GetOwnerCharacter();
    if (!Character || !ItemToDrop)
    {
        return;
    }

	if (LocallyAttachedItem == ItemToDrop)
	{
	   LocallyAttachedItem = nullptr;
    }

    ItemToDrop->DetachFromHolder(Character);

    const FVector DropLocation = GetDropLocationForItem(ItemToDrop, IgnoreIncomingItem, DropOrigin);
    ItemToDrop->SetActorLocation(DropLocation, false, nullptr, ETeleportType::TeleportPhysics);
    ItemToDrop->SetPickupEnabled(true);
    ItemToDrop->SetWorldPhysicsEnabled(true);
    if (UStaticMeshComponent* ItemMesh = ItemToDrop->GetItemMesh())
    {
        ItemMesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
        ItemMesh->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	}
	ItemToDrop->ForceNetUpdate();
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

    const FVector DropOrigin = Character->GetActorLocation() +
       Character->GetActorForwardVector() * 120.0f +
       FVector(0.0f, 0.0f, 40.0f);

    DropItemToGround(DroppedItem, nullptr, DropOrigin);
    Character->ForceNetUpdate();
}

AGProjectItemActorBase* UGProjectItemHolderComponent::FindClosestPickup() const
{
    AGProjectCharacter* Character = GetOwnerCharacter();
    UWorld* World = GetWorld();
    if (!Character || !World) return nullptr;

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

    AGProjectItemActorBase* ClosestItem = nullptr;
    float ClosestDistanceSquared = TNumericLimits<float>::Max();
    TSet<AGProjectItemActorBase*> CheckedItems;

    for (const FOverlapResult& OverlapResult : OverlapResults)
    {
       AGProjectItemActorBase* Item = Cast<AGProjectItemActorBase>(OverlapResult.GetActor());
       if (!Item || CheckedItems.Contains(Item)) continue;
       CheckedItems.Add(Item);

       const float DistanceSquared = FVector::DistSquared2D(Character->GetActorLocation(), Item->GetPickupLocation());
       const bool bCanPickup = Item->CanBePickedUpBy(Character);

       if (!bCanPickup) continue;

       if (DistanceSquared < ClosestDistanceSquared)
       {
          ClosestDistanceSquared = DistanceSquared;
          ClosestItem = Item;
       }
    }

    return ClosestItem;
}
