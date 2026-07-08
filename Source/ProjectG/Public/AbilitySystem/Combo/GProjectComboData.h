// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AbilitySystem/Combo/GProjectComboTypes.h"
#include "GProjectComboData.generated.h"

class UAnimMontage;

UCLASS(BlueprintType)
class PROJECTG_API UGProjectComboData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
	TObjectPtr<UAnimMontage> ComboMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
	TArray<FGProjectComboStep> ComboSteps;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Buffer", meta = (ClampMin = "0.05"))
	float InputBufferLifetime = 1.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Buffer", meta = (ClampMin = "1"))
	int32 MaxBufferedInputs = 8;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trace", meta = (ClampMin = "0.0"))
	float TraceRadius = 20.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trace|Debug")
	bool bDrawDebugTrace = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trace|Debug", meta = (ClampMin = "0.0"))
	float DebugTraceDuration = 1.0f;
};
