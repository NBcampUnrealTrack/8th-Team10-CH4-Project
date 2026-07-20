// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GProjectTitleWidget.generated.h"

class UButton;
class UTextBlock;
class UTexture2D;

/** Lightweight title screen that owns no references to gameplay-world objects. */
UCLASS()
class PROJECTG_API UGProjectTitleWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UGProjectTitleWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

private:
	UFUNCTION()
	void HandleExitClicked();

	void RequestStart();
	void OpenMainMenu();

	UPROPERTY(EditDefaultsOnly, Category = "Title Appearance")
	TObjectPtr<UTexture2D> BackgroundTexture;

	UPROPERTY(EditDefaultsOnly, Category = "Title Appearance")
	FLinearColor BackgroundShadeColor;

	UPROPERTY(EditDefaultsOnly, Category = "Title Appearance")
	FText TitleText;

	UPROPERTY(EditDefaultsOnly, Category = "Title Appearance")
	FText SubtitleText;

	UPROPERTY(EditDefaultsOnly, Category = "Title Appearance")
	bool bShowAccentLine = false;

	UPROPERTY(EditDefaultsOnly, Category = "Title Appearance", meta = (EditCondition = "bShowAccentLine"))
	FLinearColor AccentLineColor;

	UPROPERTY(EditDefaultsOnly, Category = "Title Buttons")
	FText StartButtonText;

	UPROPERTY(EditDefaultsOnly, Category = "Title Buttons", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float StartButtonVerticalPosition = 0.78f;

	UPROPERTY(EditDefaultsOnly, Category = "Title Buttons")
	FVector2D StartButtonSize = FVector2D(360.0f, 68.0f);

	UPROPERTY(EditDefaultsOnly, Category = "Title Buttons")
	FLinearColor StartButtonColor;

	UPROPERTY(EditDefaultsOnly, Category = "Title Buttons")
	FText ExitButtonText;

	UPROPERTY(EditDefaultsOnly, Category = "Title Buttons", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ExitButtonVerticalPosition = 0.88f;

	UPROPERTY(EditDefaultsOnly, Category = "Title Buttons")
	FVector2D ExitButtonSize = FVector2D(180.0f, 46.0f);

	UPROPERTY(EditDefaultsOnly, Category = "Title Buttons")
	FLinearColor ExitButtonColor;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> StartPromptText;

	UPROPERTY(Transient)
	TObjectPtr<UButton> ExitButton;

	bool bTransitioning = false;
	float ElapsedSeconds = 0.0f;
	FTimerHandle MenuTravelTimer;
};
