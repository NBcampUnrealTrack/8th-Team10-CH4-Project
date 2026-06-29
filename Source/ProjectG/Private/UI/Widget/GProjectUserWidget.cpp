// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/Widget/GProjectUserWidget.h"

void UGProjectUserWidget::SetWidgetController(UObject* InWidgetController)
{
	WidgetController = InWidgetController;
	NativeWidgetControllerSet();
	WidgetControllerSet();
}

void UGProjectUserWidget::NativeWidgetControllerSet()
{
}
