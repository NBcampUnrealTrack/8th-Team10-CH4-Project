// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/HUD/GProjectHUD.h"

#include "Blueprint/UserWidget.h"
#include "UI/Widget/GProjectOverlayWidget.h"
#include "UI/Widget/GProjectUserWidget.h"
#include "UI/WidgetController/GProjectOverlayWidgetController.h"
#include "UI/WidgetController/GProjectWidgetController.h"

void AGProjectHUD::InitOverlay(APlayerController* InPlayerController, APlayerState* InPlayerState, UAbilitySystemComponent* InASC, UAttributeSet* InAttributeSet)
{
	if (OverlayWidget || !OverlayWidgetClass || !OverlayWidgetControllerClass)
	{
		return;
	}

	OverlayWidgetController = NewObject<UGProjectOverlayWidgetController>(this, OverlayWidgetControllerClass);

	const FGProjectWidgetControllerParams Params(InPlayerController, InPlayerState, InASC, InAttributeSet);
	OverlayWidgetController->SetWidgetControllerParams(Params);
	OverlayWidgetController->BindCallbacksToDependencies();

	OverlayWidget = CreateWidget<UGProjectOverlayWidget>(GetWorld(), OverlayWidgetClass);
	if (OverlayWidget)
	{
		OverlayWidget->SetWidgetController(OverlayWidgetController);
		OverlayWidgetController->BroadcastInitialValues();
		OverlayWidget->AddToViewport();
	}
}
