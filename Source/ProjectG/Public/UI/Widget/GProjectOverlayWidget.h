// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/GProjectUserWidget.h"
#include "GProjectOverlayWidget.generated.h"

class UGProjectOverlayWidgetController;
class UGProjectPlayerBoxWidget;

UCLASS()
class PROJECTG_API UGProjectOverlayWidget : public UGProjectUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeWidgetControllerSet() override;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UGProjectPlayerBoxWidget> LocalPlayerBox;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UGProjectPlayerBoxWidget> OpponentPlayerBox;

private:
	UFUNCTION()
	void OnHealthChanged(float NewValue);

	UFUNCTION()
	void OnMaxHealthChanged(float NewValue);

	UFUNCTION()
	void OnSPChanged(float NewValue);

	UFUNCTION()
	void OnMaxSPChanged(float NewValue);
};
