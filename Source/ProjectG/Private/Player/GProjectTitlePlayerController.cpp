// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/GProjectTitlePlayerController.h"

#include "UI/Widget/GProjectTitleWidget.h"

AGProjectTitlePlayerController::AGProjectTitlePlayerController()
{
	TitleWidgetClass = UGProjectTitleWidget::StaticClass();
}

void AGProjectTitlePlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController())
	{
		return;
	}

	if (!TitleWidgetClass)
	{
		return;
	}

	TitleWidget = CreateWidget<UGProjectTitleWidget>(this, TitleWidgetClass);
	if (!TitleWidget)
	{
		return;
	}

	TitleWidget->AddToViewport(100);
	TitleWidget->SetKeyboardFocus();

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(TitleWidget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
	bShowMouseCursor = true;
}

void AGProjectTitlePlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (TitleWidget)
	{
		TitleWidget->RemoveFromParent();
		TitleWidget = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}
