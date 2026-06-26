// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/Widget/GProjectOverlayWidget.h"

#include "UI/Widget/GProjectPlayerBoxWidget.h"
#include "UI/WidgetController/GProjectOverlayWidgetController.h"

void UGProjectOverlayWidget::NativeWidgetControllerSet()
{
	Super::NativeWidgetControllerSet();

	UGProjectOverlayWidgetController* OverlayController = Cast<UGProjectOverlayWidgetController>(WidgetController);
	if (!OverlayController)
	{
		return;
	}

	if (LocalPlayerBox)
	{
		LocalPlayerBox->SetWidgetController(OverlayController);
	}

	OverlayController->OnHealthChanged.AddDynamic(this, &ThisClass::OnHealthChanged);
	OverlayController->OnMaxHealthChanged.AddDynamic(this, &ThisClass::OnMaxHealthChanged);
	OverlayController->OnSPChanged.AddDynamic(this, &ThisClass::OnSPChanged);
	OverlayController->OnMaxSPChanged.AddDynamic(this, &ThisClass::OnMaxSPChanged);
}

void UGProjectOverlayWidget::OnHealthChanged(float NewValue)
{
	if (LocalPlayerBox)
	{
		LocalPlayerBox->SetHealth(NewValue);
	}
}

void UGProjectOverlayWidget::OnMaxHealthChanged(float NewValue)
{
	if (LocalPlayerBox)
	{
		LocalPlayerBox->SetMaxHealth(NewValue);
	}
}

void UGProjectOverlayWidget::OnSPChanged(float NewValue)
{
	if (LocalPlayerBox)
	{
		LocalPlayerBox->SetSP(NewValue);
	}
}

void UGProjectOverlayWidget::OnMaxSPChanged(float NewValue)
{
	if (LocalPlayerBox)
	{
		LocalPlayerBox->SetMaxSP(NewValue);
	}
}
