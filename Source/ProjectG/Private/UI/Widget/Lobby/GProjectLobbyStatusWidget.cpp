// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/Lobby/GProjectLobbyStatusWidget.h"

#include "Components/TextBlock.h"

void UGProjectLobbyStatusWidget::SetSlotInfo(const FString& InPlayerName, const FString& InStatus)
{
	if (PlayerNameText) PlayerNameText->SetText(FText::FromString(InPlayerName));
	if (StatusText) StatusText->SetText(FText::FromString(InStatus));
}
