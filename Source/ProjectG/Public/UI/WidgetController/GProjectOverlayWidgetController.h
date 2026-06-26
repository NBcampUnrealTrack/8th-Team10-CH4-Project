// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/GProjectWidgetController.h"
#include "GProjectOverlayWidgetController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGProjectOnAttributeChangedSignature, float, NewValue);

UCLASS(BlueprintType, Blueprintable)
class PROJECTG_API UGProjectOverlayWidgetController : public UGProjectWidgetController
{
	GENERATED_BODY()

public:
	virtual void BroadcastInitialValues() override;
	virtual void BindCallbacksToDependencies() override;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FGProjectOnAttributeChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FGProjectOnAttributeChangedSignature OnMaxHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FGProjectOnAttributeChangedSignature OnSPChanged;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FGProjectOnAttributeChangedSignature OnMaxSPChanged;
};
