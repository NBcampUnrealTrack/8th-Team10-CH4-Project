// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/Lobby/GProjectLobbyStatusWidget.h"

#include "Components/TextBlock.h"

void UGProjectLobbyStatusWidget::UpdateStatusText(const FString& StatusStr, const FLinearColor& Color)
{
	if (StatusText)
	{
		StatusText->SetText(FText::FromString(StatusStr));
		StatusText->SetColorAndOpacity(FSlateColor(Color));
	}
}