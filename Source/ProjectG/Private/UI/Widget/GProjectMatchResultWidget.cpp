// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/GProjectMatchResultWidget.h"

#include "Components/TextBlock.h"

void UGProjectMatchResultWidget::ShowResult(
	bool bVictory,
	int32 RedTeamWins,
	int32 BlueTeamWins)
{
	SetVisibility(ESlateVisibility::Visible);

	if (ResultText)
	{
		if (bVictory)
		{
			ResultText->SetText(
				NSLOCTEXT(
					"MatchResult",
					"Victory",
					"VICTORY"
				)
			);

			ResultText->SetColorAndOpacity(
				FSlateColor(
					FLinearColor::FromSRGBColor(
						FColor(255, 225, 100, 255)
					)
				)
			);
		}
		else
		{
			ResultText->SetText(
				NSLOCTEXT(
					"MatchResult",
					"Defeat",
					"DEFEAT"
				)
			);

			ResultText->SetColorAndOpacity(
				FSlateColor(
					FLinearColor::FromSRGBColor(
						FColor(255, 110, 120, 255)
					)
				)
			);
		}
	}

	if (FinalScoreText)
	{
		FinalScoreText->SetText(
			FText::Format(
				NSLOCTEXT(
					"MatchResult",
					"FinalScore",
					"RED {0} : {1} BLUE"
				),
				FText::AsNumber(RedTeamWins),
				FText::AsNumber(BlueTeamWins)
			)
		);
	}
}

void UGProjectMatchResultWidget::HideResult()
{
	SetVisibility(ESlateVisibility::Collapsed);
}