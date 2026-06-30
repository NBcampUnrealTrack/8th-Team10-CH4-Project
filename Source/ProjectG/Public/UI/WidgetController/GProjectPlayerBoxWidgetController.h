// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/GProjectWidgetController.h"
#include "GProjectPlayerBoxWidgetController.generated.h"

struct FOnAttributeChangeData;

UCLASS(BlueprintType, Blueprintable)
class PROJECTG_API UGProjectPlayerBoxWidgetController : public UGProjectWidgetController
{
	GENERATED_BODY()

public:
	virtual void BroadcastInitialValues() override;
	virtual void BindCallbacksToDependencies() override;

	FText GetPlayerName() const;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FGProjectOnAttributeChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FGProjectOnAttributeChangedSignature OnMaxHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FGProjectOnAttributeChangedSignature OnSPChanged;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FGProjectOnAttributeChangedSignature OnMaxSPChanged;

private:
	void HealthChanged(const FOnAttributeChangeData& Data);
	void MaxHealthChanged(const FOnAttributeChangeData& Data);
	void SPChanged(const FOnAttributeChangeData& Data);
	void MaxSPChanged(const FOnAttributeChangeData& Data);
};
