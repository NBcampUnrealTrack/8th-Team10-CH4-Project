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

	PickupCollision = CreateDefaultSubobject<USphereComponent>(TEXT("PickupCollision"));
	PickupCollision->InitSphereRadius(100.0f);
	PickupCollision->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	SetRootComponent(PickupCollision);

	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	ItemMesh->SetupAttachment(PickupCollision);
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AGProjectItemActorBase::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		InitialSpawnTransform = GetActorTransform();
	}
}

void AGProjectItemActorBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ApplyDefinitionMesh();
}

bool AGProjectItemActorBase::CanBePickedUpBy(const AGProjectCharacter* Character) const
{
	return HasAuthority() &&
		Character &&
		!GetOwner() &&
		FVector::DistSquared(GetActorLocation(), Character->GetActorLocation()) <= FMath::Square(MaxPickupDistance);
}

void AGProjectItemActorBase::HandleEquipped(AGProjectCharacter* Character)
{
	SetOwner(Character);
	SetPickupEnabled(false);
}

void AGProjectItemActorBase::HandleUnequipped(AGProjectCharacter* Character)
{
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	SetOwner(nullptr);
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

void AGProjectItemActorBase::ResetToSpawnTransform()
{
	if (!HasAuthority())
	{
		return;
	}

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	SetOwner(nullptr);

	SetActorTransform(
		InitialSpawnTransform,
		false,
		nullptr,
		ETeleportType::TeleportPhysics
	);

	ApplyDefinitionMesh();

	SetPickupEnabled(true);

	SetActorHiddenInGame(false);

	ForceNetUpdate();
}

void AGProjectItemActorBase::ApplyDefinitionMesh()
{
	if (ItemDefinition && !ItemDefinition->PickupMesh.IsNull())
	{
		ItemMesh->SetStaticMesh(ItemDefinition->PickupMesh.LoadSynchronous());
	}
}
