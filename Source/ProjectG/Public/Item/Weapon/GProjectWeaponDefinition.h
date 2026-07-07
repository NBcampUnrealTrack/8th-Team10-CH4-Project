// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Item/GProjectItemDefinition.h"
#include "GProjectWeaponDefinition.generated.h"

class UGProjectComboData;
class UStaticMesh;

UCLASS(BlueprintType)
class PROJECTG_API UGProjectWeaponDefinition : public UGProjectItemDefinition
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSoftObjectPtr<UStaticMesh> HeldMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Combo")
	TObjectPtr<UGProjectComboData> GroundComboData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Combo")
	TObjectPtr<UGProjectComboData> AirComboData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Combo")
	TObjectPtr<UGProjectComboData> DashComboData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Socket")
	FName AttachSocketName = TEXT("WeaponSocket");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Socket")
	FName TraceStartSocketName = TEXT("Trace_Start");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Socket")
	FName TraceEndSocketName = TEXT("Trace_End");
};
