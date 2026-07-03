// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/GProjectUserWidget.h"
#include "GProjectOverlayWidget.generated.h"

class UGProjectOverlayWidgetController;
class UGProjectPlayerBoxWidget;
class UGProjectPlayerBoxWidgetController;
class UPanelWidget;
class UGProjectMatchTimerWidget;

UCLASS()
class PROJECTG_API UGProjectOverlayWidget : public UGProjectUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeWidgetControllerSet() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget> PlayerBoxContainer;

	UPROPERTY(EditDefaultsOnly, Category = "Player List")
	TSubclassOf<UGProjectPlayerBoxWidget> PlayerBoxWidgetClass;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UGProjectMatchTimerWidget> MatchTimerWidget;

private:
	UFUNCTION()
	void RefreshPlayerBoxes();

	UFUNCTION()
	void RefreshMatchTimer(int32 RemainTime);

	UPROPERTY()
	TArray<TObjectPtr<UGProjectPlayerBoxWidgetController>> PlayerBoxControllers;
};
