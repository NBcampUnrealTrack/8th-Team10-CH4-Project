// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/WidgetController/GProjectWidgetController.h"

#include "AbilitySystem/GProjectAbilitySystemComponent.h"
#include "AbilitySystem/GProjectAttributeSet.h"

void UGProjectWidgetController::SetWidgetControllerParams(const FGProjectWidgetControllerParams& InParams)
{
	PlayerController = InParams.PlayerController;
	PlayerState = InParams.PlayerState;
	AbilitySystemComponent = InParams.AbilitySystemComponent;
	AttributeSet = InParams.AttributeSet;
}

void UGProjectWidgetController::BroadcastInitialValues()
{
}

void UGProjectWidgetController::BindCallbacksToDependencies()
{
}

UGProjectAbilitySystemComponent* UGProjectWidgetController::GetGProjectASC()
{
	if (!GProjectAbilitySystemComponent)
	{
		GProjectAbilitySystemComponent = Cast<UGProjectAbilitySystemComponent>(AbilitySystemComponent);
	}

	return GProjectAbilitySystemComponent;
}

UGProjectAttributeSet* UGProjectWidgetController::GetGProjectAS()
{
	if (!GProjectAttributeSet)
	{
		GProjectAttributeSet = Cast<UGProjectAttributeSet>(AttributeSet);
	}

	return GProjectAttributeSet;
}
