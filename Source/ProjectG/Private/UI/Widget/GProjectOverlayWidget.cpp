// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/Widget/GProjectOverlayWidget.h"

#include "AbilitySystem/GProjectAbilitySystemComponent.h"
#include "AbilitySystem/GProjectAttributeSet.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Character/GProjectCharacter.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "Player/GProjectPlayerState.h"
#include "Targeting/GProjectLockOnComponent.h"
#include "UI/Widget/GProjectPlayerBoxWidget.h"
#include "UI/WidgetController/GProjectOverlayWidgetController.h"
#include "UI/WidgetController/GProjectPlayerBoxWidgetController.h"
#include "UI/WidgetController/GProjectWidgetController.h"
#include "UI/Widget/GProjectMatchTimerWidget.h"
#include "UI/Widget/GProjectChatWidget.h"

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

	BindLockOnComponent();

	OverlayController->OnChatMessageReceived.RemoveDynamic(this, &ThisClass::RefreshChatMessage);
	OverlayController->OnChatMessageReceived.AddDynamic(this, &ThisClass::RefreshChatMessage);


}

void UGProjectOverlayWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	UpdateLockOnIndicator();
}

void UGProjectOverlayWidget::NativeDestruct()
{
	if (BoundLockOnComponent)
	{
		BoundLockOnComponent->OnLockOnTargetChanged.RemoveDynamic(
			this, &ThisClass::OnLockOnTargetChanged);
	}

	Super::NativeDestruct();

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

void UGProjectOverlayWidget::BindLockOnComponent()
{
	if (BoundLockOnComponent)
	{
		BoundLockOnComponent->OnLockOnTargetChanged.RemoveDynamic(
			this, &ThisClass::OnLockOnTargetChanged);
	}

	const AGProjectCharacter* Character = Cast<AGProjectCharacter>(GetOwningPlayerPawn());
	BoundLockOnComponent = Character ? Character->GetLockOnComponent() : nullptr;
	if (!BoundLockOnComponent)
	{
		OnLockOnTargetChanged(nullptr);
		return;
	}

	BoundLockOnComponent->OnLockOnTargetChanged.AddDynamic(
		this, &ThisClass::OnLockOnTargetChanged);
	OnLockOnTargetChanged(BoundLockOnComponent->GetCurrentTarget());
}

void UGProjectOverlayWidget::OnLockOnTargetChanged(AActor* NewTarget)
{
	LockOnTarget = NewTarget;

	if (LockOnIndicator)
	{
		LockOnIndicator->SetVisibility(
			LockOnTarget ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
}

void UGProjectOverlayWidget::RefreshChatMessage(int32 SenderPlayerID, const FString& SenderName, const FString& Message)
{
	if (!ChatWidget)
	{
		return;
	}

	ChatWidget->AddChatMessage(SenderPlayerID, SenderName, Message);
}

void UGProjectOverlayWidget::UpdateLockOnIndicator()
{
	if (!LockOnIndicator)
	{
		return;
	}

	if (!IsValid(LockOnTarget))
	{
		LockOnIndicator->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	FVector2D WidgetPosition;
	const FVector WorldPosition = LockOnTarget->GetActorLocation() + FVector::UpVector * IndicatorHeightOffset;
	const bool bProjected = UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(
		GetOwningPlayer(),
		WorldPosition,
		WidgetPosition,
		true);

	UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(LockOnIndicator->Slot);
	if (!bProjected || !CanvasSlot)
	{
		LockOnIndicator->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	CanvasSlot->SetPosition(WidgetPosition);
	CanvasSlot->SetAlignment(FVector2D(0.5f));
	LockOnIndicator->SetVisibility(ESlateVisibility::HitTestInvisible);
}