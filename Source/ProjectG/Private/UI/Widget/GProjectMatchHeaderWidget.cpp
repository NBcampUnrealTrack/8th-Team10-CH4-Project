// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/GProjectMatchHeaderWidget.h"

#include "Components/TextBlock.h"

void UGProjectMatchHeaderWidget::SetTeamScore(
	int32 RedTeamWins,
	int32 BlueTeamWins)
{
	if (RedScoreText)
	{
		RedScoreText->SetText(
			FText::AsNumber(RedTeamWins)
		);
	}

	if (BlueScoreText)
	{
		BlueScoreText->SetText(
			FText::AsNumber(BlueTeamWins)
		);
	}
}

void UGProjectMatchHeaderWidget::SetRemainTime(
	int32 RemainTime)
{
	if (!TimerText)
	{
		return;
	}

	TimerText->SetText(
		FText::FromString(
			FString::Printf(
				TEXT("%d"),
				RemainTime
			)
		)
	);
}