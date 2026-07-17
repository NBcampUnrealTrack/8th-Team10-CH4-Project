// Copyright Epic Games, Inc. All Rights Reserved.

#include "Item/GProjectItemActorBase.h"

#include "Character/GProjectCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Item/GProjectItemDefinition.h"
#include "Net/UnrealNetwork.h"

AGProjectItemActorBase::AGProjectItemActorBase()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	bReplicates = true;
	SetReplicateMovement(true);

	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	ItemMesh->SetIsReplicated(true);
	ItemMesh->SetCollisionProfileName(TEXT("NoCollision"));
	SetRootComponent(ItemMesh);

	PickupCollision = CreateDefaultSubobject<USphereComponent>(TEXT("PickupCollision"));
	PickupCollision->InitSphereRadius(70.0f);
	PickupCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PickupCollision->SetCollisionObjectType(ECC_WorldDynamic);
	PickupCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	PickupCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	PickupCollision->SetGenerateOverlapEvents(true);
	PickupCollision->SetupAttachment(ItemMesh);
}

void AGProjectItemActorBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGProjectItemActorBase, bWorldPhysicsEnabled);
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

void AGProjectItemActorBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!HasAuthority() || !bWorldPhysicsEnabled || !ItemMesh || !ItemMesh->IsSimulatingPhysics())
	{
		SetActorTickEnabled(false);
		return;
	}

	const FTransform PhysicsTransform = ItemMesh->GetComponentTransform();
	SetActorTransform(
		PhysicsTransform,
		false,
		nullptr,
		ETeleportType::TeleportPhysics);

	const float LinearSpeed = ItemMesh->GetPhysicsLinearVelocity().Size();
	const float AngularSpeed = ItemMesh->GetPhysicsAngularVelocityInDegrees().Size();
	if (!ItemMesh->RigidBodyIsAwake(NAME_None) &&
		LinearSpeed <= PhysicsSleepLinearSpeed &&
		AngularSpeed <= PhysicsSleepAngularSpeed)
	{
		FinalizeWorldPhysicsTransform();
		SetActorTickEnabled(false);
		return;
	}

	ForceNetUpdate();
}

void AGProjectItemActorBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ApplyItemMesh();
}

bool AGProjectItemActorBase::CanBePickedUpBy(const AGProjectCharacter* Character) const
{
	if (const UWorld* World = GetWorld())
	{
		if (World->GetTimeSeconds() < PickupBlockedUntilTime)
		{
			return false;
		}
	}

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

	if (HasAuthority())
	{
		SetOwner(Character);
		SetReplicateMovement(false);
		bWorldPhysicsEnabled = false;
		ApplyServerWorldPhysics(false);
		SetPickupEnabled(false);
	}
	SetActorHiddenInGame(false);
	if (!HasAuthority())
	{
		ApplyClientWorldVisualState(false);
	}

	AttachItemVisual(Character, HoldSocketName);
}

void AGProjectItemActorBase::HandleUnequipped(AGProjectCharacter* Character)
{
	DetachFromHolder(Character);
	if (HasAuthority())
	{
		bWorldPhysicsEnabled = true;
		ApplyServerWorldPhysics(true);
	}
}

void AGProjectItemActorBase::DetachFromHolder(AGProjectCharacter* Character)
{
	DetachItemVisual();
	if (HasAuthority())
	{
		SetOwner(nullptr);
		SetReplicateMovement(true);
		bWorldPhysicsEnabled = false;
		ApplyServerWorldPhysics(false);
	}
}

bool AGProjectItemActorBase::Use_Implementation(AGProjectCharacter* Character)
{
	return false;
}

void AGProjectItemActorBase::SetPickupEnabled(bool bEnabled)
{
	if (PickupCollision)
	{
		PickupCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
		PickupCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	}

	PickupCollision->SetCollisionEnabled(
		bEnabled ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	ItemMesh->SetVisibility(true, true);
}

void AGProjectItemActorBase::BlockPickupFor(float Duration)
{
	if (!HasAuthority() || Duration <= 0.0f)
	{
		return;
	}

	if (const UWorld* World = GetWorld())
	{
		PickupBlockedUntilTime = FMath::Max(PickupBlockedUntilTime, World->GetTimeSeconds() + Duration);
	}
}

void AGProjectItemActorBase::BlockPickupBriefly()
{
	BlockPickupFor(DefaultPickupBlockDuration);
}

void AGProjectItemActorBase::SetWorldPhysicsEnabled(bool bEnabled)
{
	if (HasAuthority())
	{
		bWorldPhysicsEnabled = bEnabled;
		ApplyServerWorldPhysics(bEnabled);
		return;
	}

	ApplyClientWorldVisualState(bEnabled);
}

void AGProjectItemActorBase::ApplyServerWorldPhysics(bool bEnabled)
{
	if (PickupCollision)
	{
		PickupCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
		PickupCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		PickupCollision->SetCollisionEnabled(
			bEnabled ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	}

	if (ItemMesh)
	{
		ItemMesh->SetSimulatePhysics(bEnabled);
		ItemMesh->SetEnableGravity(bEnabled);
		if (bEnabled)
		{
			SetReplicateMovement(true);
			ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			ItemMesh->SetCollisionObjectType(ECC_PhysicsBody);
			ItemMesh->SetCollisionResponseToAllChannels(ECR_Block);
			ItemMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
			ItemMesh->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Ignore);
			ItemMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
			ItemMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
			ItemMesh->WakeAllRigidBodies();
			SetActorTickEnabled(true);
		}
		else
		{
			SetActorTickEnabled(false);
			ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void AGProjectItemActorBase::FinalizeWorldPhysicsTransform()
{
	if (!HasAuthority() || !bWorldPhysicsEnabled || !ItemMesh || !ItemMesh->IsSimulatingPhysics())
	{
		return;
	}

	const FTransform FinalTransform = ItemMesh->GetComponentTransform();

	ItemMesh->SetSimulatePhysics(false);
	ItemMesh->SetEnableGravity(false);
	SetActorTransform(
		FinalTransform,
		false,
		nullptr,
		ETeleportType::TeleportPhysics);

	SetReplicateMovement(true);
	ForceNetUpdate();
}

void AGProjectItemActorBase::OnRep_WorldPhysicsEnabled()
{
	ApplyClientWorldVisualState(bWorldPhysicsEnabled);
	if (bWorldPhysicsEnabled)
	{
		SetReplicateMovement(true);
	}
}

void AGProjectItemActorBase::ApplyClientWorldVisualState(bool bWorldState)
{
	if (PickupCollision)
	{
		PickupCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
		PickupCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		PickupCollision->SetCollisionEnabled(
			bWorldState ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	}

	if (!ItemMesh)
	{
		return;
	}

	ItemMesh->SetSimulatePhysics(false);
	ItemMesh->SetEnableGravity(false);
	if (bWorldState)
	{
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		ItemMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	}
	else
	{
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	ItemMesh->SetVisibility(true, true);
}

void AGProjectItemActorBase::AttachItemVisual(AGProjectCharacter* Character, FName HoldSocketName)
{
	if (!Character)
	{
		return;
	}

	SetActorHiddenInGame(false);
	AttachToComponent(
		Character->GetMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		HoldSocketName);
}

void AGProjectItemActorBase::DetachItemVisual()
{
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	SetActorHiddenInGame(false);
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
