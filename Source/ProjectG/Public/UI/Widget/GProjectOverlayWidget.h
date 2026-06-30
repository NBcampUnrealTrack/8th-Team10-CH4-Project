// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/GProjectUserWidget.h"
#include "GProjectOverlayWidget.generated.h"

class UGProjectOverlayWidgetController;
class UGProjectPlayerBoxWidget;
class UGProjectPlayerBoxWidgetController;
class UPanelWidget;

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

private:
	UFUNCTION()
	void RefreshPlayerBoxes();

	UPROPERTY()
	TArray<TObjectPtr<UGProjectPlayerBoxWidgetController>> PlayerBoxControllers;
};
