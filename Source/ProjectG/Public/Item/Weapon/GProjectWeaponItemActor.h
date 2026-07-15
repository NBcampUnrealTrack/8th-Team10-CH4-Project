// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Item/GProjectItemActorBase.h"
#include "GProjectWeaponItemActor.generated.h"

class UGProjectWeaponDefinition;

UCLASS()
class PROJECTG_API AGProjectWeaponItemActor : public AGProjectItemActorBase
{
	GENERATED_BODY()

public:
	virtual bool CanBePickedUpBy(const AGProjectCharacter* Character) const override;
	virtual bool UsesWeaponSocket() const override;
	virtual void HandleEquipped(AGProjectCharacter* Character, FName HoldSocketName) override;
	virtual void HandleUnequipped(AGProjectCharacter* Character) override;

private:
	const UGProjectWeaponDefinition* GetWeaponDefinition() const;
};
