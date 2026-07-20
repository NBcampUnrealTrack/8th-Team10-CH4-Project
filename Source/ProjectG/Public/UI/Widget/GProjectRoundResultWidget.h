// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Game/GProjectGameState.h"
#include "GProjectRoundResultWidget.generated.h"

class UTextBlock;
class UWidgetAnimation;

UCLASS()
class PROJECTG_API UGProjectRoundResultWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void ShowRoundResult(const FGProjectRoundResultData& RoundResultData);

	void HideRoundResult();

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ResultText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ReasonText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TeamHPText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ScoreNoticeText;



	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> PanelMoveAnimation;

private:
	FText MakeResultText(ERoundResult Result) const;

	FText MakeReasonText(ERoundEndReason Reason) const;

};

