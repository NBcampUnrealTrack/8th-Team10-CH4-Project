// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/Lobby/GProjectLobbyStatusWidget.h"

#include "Components/TextBlock.h"

void UGProjectLobbyStatusWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	const FLinearColor PrimaryText(0.86f, 0.94f, 1.0f, 1.0f);
	const FLinearColor SecondaryText(0.36f, 0.78f, 0.92f, 1.0f);
	const FLinearColor Outline(0.0f, 0.02f, 0.05f, 0.92f);

	auto StyleText = [&Outline](UTextBlock* TextBlock, const FLinearColor& Color, int32 Size)
	{
		if (!TextBlock)
		{
			return;
		}

		TextBlock->SetColorAndOpacity(FSlateColor(Color));
		TextBlock->SetJustification(ETextJustify::Center);
		TextBlock->SetShadowOffset(FVector2D(1.5f, 2.0f));
		TextBlock->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.65f));

		FSlateFontInfo Font = TextBlock->GetFont();
		Font.Size = Size;
		Font.OutlineSettings.OutlineSize = 1;
		Font.OutlineSettings.OutlineColor = Outline;
		TextBlock->SetFont(Font);
	};

	StyleText(PlayerNameText, PrimaryText, 22);
	StyleText(StatusText, SecondaryText, 17);
}

void UGProjectLobbyStatusWidget::SetSlotInfo(const FString& InPlayerName, const FString& InStatus)
{
	if (PlayerNameText)
	{
		PlayerNameText->SetText(FText::FromString(InPlayerName));
		PlayerNameText->SetColorAndOpacity(FSlateColor(
			InPlayerName.Contains(TEXT("Empty"))
				? FLinearColor(0.42f, 0.50f, 0.56f, 0.8f)
				: FLinearColor(0.86f, 0.94f, 1.0f, 1.0f)));
	}

	if (StatusText)
	{
		StatusText->SetText(FText::FromString(InStatus));

		FLinearColor StatusColor(0.50f, 0.62f, 0.70f, 1.0f);
		if (InStatus.Equals(TEXT("READY"), ESearchCase::IgnoreCase))
		{
			StatusColor = FLinearColor(0.12f, 0.92f, 0.68f, 1.0f);
		}
		else if (InStatus.Equals(TEXT("MASTER"), ESearchCase::IgnoreCase))
		{
			StatusColor = FLinearColor(1.0f, 0.72f, 0.18f, 1.0f);
		}
		else if (InStatus.Equals(TEXT("WAIT"), ESearchCase::IgnoreCase))
		{
			StatusColor = FLinearColor(0.18f, 0.70f, 0.94f, 1.0f);
		}

		StatusText->SetColorAndOpacity(FSlateColor(StatusColor));
	}
}
