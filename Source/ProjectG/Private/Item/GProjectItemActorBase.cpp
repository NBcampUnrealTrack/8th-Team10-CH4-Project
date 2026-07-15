// Copyright Epic Games, Inc. All Rights Reserved.

#include "Item/GProjectItemActorBase.h"

#include "Character/GProjectCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Item/GProjectItemDefinition.h"

AGProjectItemActorBase::AGProjectItemActorBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	ItemMesh->SetCollisionProfileName(TEXT("NoCollision"));
	SetRootComponent(ItemMesh);

	PickupCollision = CreateDefaultSubobject<USphereComponent>(TEXT("PickupCollision"));
	PickupCollision->InitSphereRadius(100.0f);
	PickupCollision->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	PickupCollision->SetGenerateOverlapEvents(true);
	PickupCollision->SetupAttachment(ItemMesh);
}

void AGProjectItemActorBase::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		InitialSpawnTransform = GetActorTransform();
		SetWorldPhysicsEnabled(true);
	}
}

void AGProjectItemActorBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ApplyItemMesh();
}

bool AGProjectItemActorBase::CanBePickedUpBy(const AGProjectCharacter* Character) const
{
	return Character &&
		!GetOwner() &&
		FVector::DistSquared2D(GetPickupLocation(), Character->GetActorLocation()) <= FMath::Square(MaxPickupDistance);
}

FVector AGProjectItemActorBase::GetPickupLocation() const
{
	return ItemMesh ? ItemMesh->GetComponentLocation() : GetActorLocation();
}

bool AGProjectItemActorBase::CanUse(const AGProjectCharacter* Character) const
{
	return false;
}

bool AGProjectItemActorBase::UsesWeaponSocket() const
{
	return false;
}

bool AGProjectItemActorBase::ShouldDestroyOnUse() const
{
	return true;
}

bool AGProjectItemActorBase::CanBeThrown() const
{
	return true;
}

bool AGProjectItemActorBase::ShouldApplyThrowImpactDamage() const
{
	return true;
}

void AGProjectItemActorBase::OnThrowStarted(AGProjectCharacter* Thrower)
{
}

void AGProjectItemActorBase::OnThrowLanded()
{
	SetPickupEnabled(true);
}

void AGProjectItemActorBase::HandleEquipped(AGProjectCharacter* Character, FName HoldSocketName)
{
	if (!Character)
	{
		return;
	}

	SetOwner(Character);
	SetReplicateMovement(false);
	SetActorHiddenInGame(false);
	SetWorldPhysicsEnabled(false);
	SetPickupEnabled(false);

	AttachToComponent(
		Character->GetMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		HoldSocketName);
}

void AGProjectItemActorBase::HandleUnequipped(AGProjectCharacter* Character)
{
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	SetOwner(nullptr);
	SetReplicateMovement(true);
	SetWorldPhysicsEnabled(true);
}

bool AGProjectItemActorBase::Use_Implementation(AGProjectCharacter* Character)
{
	return false;
}

void AGProjectItemActorBase::SetPickupEnabled(bool bEnabled)
{
	PickupCollision->SetCollisionEnabled(
		bEnabled ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	ItemMesh->SetVisibility(true, true);
}

void AGProjectItemActorBase::SetWorldPhysicsEnabled(bool bEnabled)
{
	if (PickupCollision)
	{
		PickupCollision->SetCollisionEnabled(
			bEnabled ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	}

	if (ItemMesh)
	{
		ItemMesh->SetSimulatePhysics(bEnabled);
		ItemMesh->SetEnableGravity(bEnabled);
		if (bEnabled)
		{
			ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			ItemMesh->SetCollisionObjectType(ECC_PhysicsBody);
			ItemMesh->SetCollisionResponseToAllChannels(ECR_Block);
			ItemMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
			ItemMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
			ItemMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
		}
		else
		{
			ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void AGProjectItemActorBase::ResetToSpawnTransform()
{
	if (!HasAuthority())
	{
		return;
	}

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	SetOwner(nullptr);
	SetWorldPhysicsEnabled(false);

	SetActorTransform(
		InitialSpawnTransform,
		false,
		nullptr,
		ETeleportType::TeleportPhysics
	);

	ApplyItemMesh();

	SetWorldPhysicsEnabled(true);
	SetPickupEnabled(true);

	SetActorHiddenInGame(false);

	ForceNetUpdate();
}

void AGProjectItemActorBase::ApplyItemMesh()
{
	if (!PickupMesh.IsNull())
	{
		ItemMesh->SetStaticMesh(PickupMesh.LoadSynchronous());
		return;
	}

	if (ItemDefinition && !ItemDefinition->PickupMesh.IsNull())
	{
		ItemMesh->SetStaticMesh(ItemDefinition->PickupMesh.LoadSynchronous());
	}
}
