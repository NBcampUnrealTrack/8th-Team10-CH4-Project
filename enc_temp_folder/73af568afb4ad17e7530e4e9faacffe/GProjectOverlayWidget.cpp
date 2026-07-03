// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/Widget/GProjectOverlayWidget.h"

#include "AbilitySystem/GProjectAbilitySystemComponent.h"
#include "AbilitySystem/GProjectAttributeSet.h"
#include "Components/PanelWidget.h"
#include "Player/GProjectPlayerState.h"
#include "UI/Widget/GProjectPlayerBoxWidget.h"
#include "UI/WidgetController/GProjectOverlayWidgetController.h"
#include "UI/WidgetController/GProjectPlayerBoxWidgetController.h"
#include "UI/WidgetController/GProjectWidgetController.h"
#include "UI/Widget/GProjectMatchTimerWidget.h"

void UGProjectOverlayWidget::NativeWidgetControllerSet()
{
	Super::NativeWidgetControllerSet();

	UGProjectOverlayWidgetController* OverlayController = Cast<UGProjectOverlayWidgetController>(WidgetController);
	if (!OverlayController)
	{
		return;
	}

	OverlayController->OnPlayerListChanged.RemoveDynamic(this, &ThisClass::RefreshPlayerBoxes);
	OverlayController->OnPlayerListChanged.AddDynamic(this, &ThisClass::RefreshPlayerBoxes);
	RefreshPlayerBoxes();

	OverlayController->OnMatchTimeChanged.RemoveDynamic(this, &ThisClass::RefreshMatchTimer);
	OverlayController->OnMatchTimeChanged.AddDynamic(this, &ThisClass::RefreshMatchTimer);
}

void UGProjectOverlayWidget::RefreshPlayerBoxes()
{
	UGProjectOverlayWidgetController* OverlayController = Cast<UGProjectOverlayWidgetController>(WidgetController);
	if (!OverlayController || !PlayerBoxContainer || !PlayerBoxWidgetClass)
	{
		return;
	}

	PlayerBoxContainer->ClearChildren();
	PlayerBoxControllers.Reset();

	for (AGProjectPlayerState* CurrentPlayerState : OverlayController->GetOrderedPlayerStates())
	{
		if (!CurrentPlayerState || !CurrentPlayerState->GetGProjectAbilitySystemComponent() || !CurrentPlayerState->GetAttributeSet())
		{
			continue;
		}

		UGProjectPlayerBoxWidgetController* BoxController = NewObject<UGProjectPlayerBoxWidgetController>(this);
		const FGProjectWidgetControllerParams Params(
			GetOwningPlayer(),
			CurrentPlayerState,
			CurrentPlayerState->GetGProjectAbilitySystemComponent(),
			CurrentPlayerState->GetAttributeSet());
		BoxController->SetWidgetControllerParams(Params);
		BoxController->BindCallbacksToDependencies();

		UGProjectPlayerBoxWidget* PlayerBox = CreateWidget<UGProjectPlayerBoxWidget>(
			GetOwningPlayer(), PlayerBoxWidgetClass);
		if (!PlayerBox)
		{
			continue;
		}

		PlayerBox->SetWidgetController(BoxController);
		PlayerBoxContainer->AddChild(PlayerBox);
		BoxController->BroadcastInitialValues();
		PlayerBoxControllers.Add(BoxController);
	}
}

void UGProjectOverlayWidget::RefreshMatchTimer(int32 RemainTime)
{
	if (MatchTimerWidget)
	{
		MatchTimerWidget->SetRemainTime(RemainTime);
	}
}
