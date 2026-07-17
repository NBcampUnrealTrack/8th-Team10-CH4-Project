// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/GProjectRoundResultWidget.h"

#include "Components/TextBlock.h"

void UGProjectRoundResultWidget::NativeConstruct()
{
	Super::NativeConstruct();

	HideRoundResult();
}

void UGProjectRoundResultWidget::ShowRoundResult(const FGProjectRoundResultData& RoundResultData)
{
	if (ResultText)
	{
		ResultText->SetText(MakeResultText(RoundResultData.Result));
	}

	if (ReasonText)
	{
		ReasonText->SetText(MakeReasonText(RoundResultData.Reason));
	}

	if (TeamHPText)
	{
		TeamHPText->SetText(
			FText::FromString(
				FString::Printf(
					TEXT("Red HP %.0f  :  Blue HP %.0f"),
					RoundResultData.RedTeamTotalHP,
					RoundResultData.BlueTeamTotalHP
				)
			)
		);
	}

	if (ScoreNoticeText)
	{
		const FString ScoreText =
			RoundResultData.Result == ERoundResult::Draw
			? TEXT("Draw - No Score")
			: FString::Printf(
				TEXT("Round Score  Red %d : %d Blue"),
				RoundResultData.RedTeamRoundWins,
				RoundResultData.BlueTeamRoundWins
			);

		ScoreNoticeText->SetText(
			FText::FromString(ScoreText)
		);
	}

	SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UGProjectRoundResultWidget::HideRoundResult()
{
	SetVisibility(ESlateVisibility::Collapsed);
}

FText UGProjectRoundResultWidget::MakeResultText(const ERoundResult Result) const
{
	switch (Result)
	{
	case ERoundResult::RedWin:
		return FText::FromString(TEXT("RED TEAM WIN!"));

	case ERoundResult::BlueWin:
		return FText::FromString(TEXT("BLUE TEAM WIN!"));

	case ERoundResult::Draw:
		return FText::FromString(TEXT("DRAW!"));

	default:
		return FText::GetEmpty();
	}
}

FText UGProjectRoundResultWidget::MakeReasonText(const ERoundEndReason Reason) const
{
	switch (Reason)
	{
	case ERoundEndReason::Elimination:
		return FText::FromString(TEXT("Team Eliminated"));

	case ERoundEndReason::TimeUp:
		return FText::FromString(TEXT("Time Up"));

	default:
		return FText::GetEmpty();
	}
}