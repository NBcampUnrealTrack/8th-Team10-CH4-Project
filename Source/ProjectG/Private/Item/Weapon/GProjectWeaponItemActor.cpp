// Copyright Epic Games, Inc. All Rights Reserved.

#include "Item/Weapon/GProjectWeaponItemActor.h"

#include "Character/GProjectCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Item/Weapon/GProjectCombatStyle.h"
#include "Item/Weapon/GProjectWeaponDefinition.h"

bool AGProjectWeaponItemActor::CanBePickedUpBy(const AGProjectCharacter* Character) const
{
	const UGProjectWeaponDefinition* WeaponDefinition = GetWeaponDefinition();

	return Character
		&& WeaponDefinition
		&& WeaponDefinition->RequiredCharacterVariant == Character->GetCharacterVariant()
		&& Super::CanBePickedUpBy(Character);
}

bool AGProjectWeaponItemActor::UsesWeaponSocket() const
{
	return true;
}

void AGProjectWeaponItemActor::HandleEquipped(AGProjectCharacter* Character, FName HoldSocketName)
{
	if (!Character)
	{
		return;
	}

	const UGProjectWeaponDefinition* WeaponDefinition = GetWeaponDefinition();

	Super::HandleEquipped(Character, HoldSocketName);

	if (!WeaponDefinition)
	{
		Character->ResetActiveComboData();
		Character->ResetAttackTraceSource();
		Character->SetCombatStyle(EGProjectCombatStyle::Unarmed);
		return;
	}

	if (!WeaponDefinition->HeldMesh.IsNull())
	{
		ItemMesh->SetStaticMesh(WeaponDefinition->HeldMesh.LoadSynchronous());
	}

	ItemMesh->SetRelativeRotation(WeaponDefinition->AttachRotationOffset);

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
	ApplyItemMesh();
}

const UGProjectWeaponDefinition* AGProjectWeaponItemActor::GetWeaponDefinition() const
{
	return Cast<UGProjectWeaponDefinition>(ItemDefinition);
}
