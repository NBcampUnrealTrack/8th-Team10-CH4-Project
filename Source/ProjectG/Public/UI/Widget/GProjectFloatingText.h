// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GProjectFloatingText.generated.h"

UCLASS()
class PROJECTG_API AGProjectFloatingText : public AActor
{
	GENERATED_BODY()

public:
	AGProjectFloatingText();

	UFUNCTION(BlueprintNativeEvent, Category = "Damage")
	void SetDamageAmount(float Damage, bool bIsCritical);
	virtual void SetDamageAmount_Implementation(float Damage, bool bIsCritical) {}
};
