// Copyright Epic Games, Inc. All Rights Reserved.

#include "Item/Weapon/GProjectWeaponItemActor.h"

#include "Character/GProjectCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Item/Weapon/GProjectWeaponDefinition.h"

bool AGProjectWeaponItemActor::CanBePickedUpBy(const AGProjectCharacter* Character) const
{
	UGProjectWeaponDefinition* WeaponDefinition = GetWeaponDefinition();
	return WeaponDefinition
		&& Character
		&& WeaponDefinition->RequiredCharacterVariant == Character->GetCharacterVariant()
		&& Super::CanBePickedUpBy(Character);
}

void AGProjectWeaponItemActor::HandleEquipped(AGProjectCharacter* Character)
{
	UGProjectWeaponDefinition* WeaponDefinition = GetWeaponDefinition();
	if (!Character || !WeaponDefinition)
	{
		return;
	}

	Super::HandleEquipped(Character);

	if (!WeaponDefinition->HeldMesh.IsNull())
	{
		ItemMesh->SetStaticMesh(WeaponDefinition->HeldMesh.LoadSynchronous());
	}

	AttachToComponent(
		Character->GetMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		WeaponDefinition->AttachSocketName);

	Character->SetActiveComboData(
		WeaponDefinition->GroundComboData,
		WeaponDefinition->AirComboData,
		WeaponDefinition->DashComboData);
	Character->SetCombatStyle(WeaponDefinition->CombatStyle);
	Character->SetAttackTraceSource(
		ItemMesh,
		WeaponDefinition->TraceStartSocketName,
		WeaponDefinition->TraceEndSocketName);
}

void AGProjectWeaponItemActor::HandleUnequipped(AGProjectCharacter* Character)
{
	if (Character)
	{
		Character->ResetActiveComboData();
		Character->ResetAttackTraceSource();
		Character->SetCombatStyle(EGProjectCombatStyle::Unarmed);
	}

	Super::HandleUnequipped(Character);
	ApplyDefinitionMesh();
}

UGProjectWeaponDefinition* AGProjectWeaponItemActor::GetWeaponDefinition() const
{
	return Cast<UGProjectWeaponDefinition>(ItemDefinition);
}
