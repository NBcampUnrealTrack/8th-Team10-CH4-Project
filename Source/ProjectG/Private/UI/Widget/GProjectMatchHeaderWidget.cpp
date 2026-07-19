// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/GProjectMatchHeaderWidget.h"

#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UGProjectMatchHeaderWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (!GetClass()->GetName().Contains(TEXT("WBP_MatchHeaderWidget_New")))
	{
		return;
	}

	const FLinearColor RedTeam(1.0f, 0.28f, 0.34f, 1.0f);
	const FLinearColor BlueTeam(0.18f, 0.66f, 1.0f, 1.0f);
	const FLinearColor PrimaryText(0.90f, 0.96f, 1.0f, 1.0f);
	const FLinearColor Outline(0.0f, 0.02f, 0.05f, 0.95f);

	auto StyleText = [&Outline](UTextBlock* Text, const FLinearColor& Color, int32 Size)
	{
		if (!Text)
		{
			return;
		}

		Text->SetColorAndOpacity(FSlateColor(Color));
		Text->SetJustification(ETextJustify::Center);
		Text->SetShadowOffset(FVector2D(1.5f, 2.0f));
		Text->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.72f));

		FSlateFontInfo Font = Text->GetFont();
		Font.Size = Size;
		Font.OutlineSettings.OutlineSize = 1;
		Font.OutlineSettings.OutlineColor = Outline;
		Text->SetFont(Font);
	};

	StyleText(RedLabelText, RedTeam, 14);
	StyleText(BlueLabelText, BlueTeam, 14);
	StyleText(RedScoreText, RedTeam, 30);
	StyleText(BlueScoreText, BlueTeam, 30);
	StyleText(TimerText, PrimaryText, 28);

	if (HeaderBackplate)
	{
		HeaderBackplate->SetBrushColor(FLinearColor(0.015f, 0.045f, 0.075f, 0.94f));
	}

	if (HeaderTopLine)
	{
		HeaderTopLine->SetColorAndOpacity(FLinearColor(0.08f, 0.72f, 0.92f, 0.95f));
	}
}

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

	if (GetClass()->GetName().Contains(TEXT("WBP_MatchHeaderWidget_New")))
	{
		const FLinearColor TimerColor = RemainTime <= 10
			? FLinearColor(1.0f, 0.24f, 0.20f, 1.0f)
			: FLinearColor(0.90f, 0.96f, 1.0f, 1.0f);
		TimerText->SetColorAndOpacity(FSlateColor(TimerColor));
	}
}
