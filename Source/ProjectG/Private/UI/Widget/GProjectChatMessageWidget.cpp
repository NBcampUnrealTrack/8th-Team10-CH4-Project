// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/GProjectChatMessageWidget.h"

#include "Components/TextBlock.h"

void UGProjectChatMessageWidget::SetMessage(const FString& SenderName, const FString& Message, const FLinearColor& SenderColor)
{
	if (!SenderNameText || !MessageText)
	{
		return;
	}

	SenderNameText->SetText(
		FText::FromString(SenderName + TEXT(":"))
	);

	SenderNameText->SetColorAndOpacity(FSlateColor(SenderColor));

	SenderNameText->SetRenderOpacity(1.0f);
	SenderNameText->SetVisibility(ESlateVisibility::Visible);

	MessageText->SetText(FText::FromString(Message));

}
