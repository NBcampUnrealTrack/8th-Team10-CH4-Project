// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/GProjectUserWidget.h"
#include "GProjectOverlayWidget.generated.h"

class AActor;
class UGProjectLockOnComponent;
class UGProjectOverlayWidgetController;
class UGProjectPlayerBoxWidget;
class UGProjectPlayerBoxWidgetController;
class UImage;
class UPanelWidget;

UCLASS()
class PROJECTG_API UGProjectOverlayWidget : public UGProjectUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeWidgetControllerSet() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget> PlayerBoxContainer;

	UPROPERTY(EditDefaultsOnly, Category = "Player List")
	TSubclassOf<UGProjectPlayerBoxWidget> PlayerBoxWidgetClass;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> LockOnIndicator;

	UPROPERTY(EditDefaultsOnly, Category = "Lock On")
	float IndicatorHeightOffset = 120.0f;

private:
	UFUNCTION()
	void RefreshPlayerBoxes();

	UFUNCTION()
	void OnLockOnTargetChanged(AActor* NewTarget);

	void BindLockOnComponent();
	void UpdateLockOnIndicator();

	UPROPERTY()
	TArray<TObjectPtr<UGProjectPlayerBoxWidgetController>> PlayerBoxControllers;

	UPROPERTY()
	TObjectPtr<UGProjectLockOnComponent> BoundLockOnComponent;

	UPROPERTY()
	TObjectPtr<AActor> LockOnTarget;
};
