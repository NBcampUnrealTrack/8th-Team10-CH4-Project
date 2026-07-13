// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/GProjectLobbyWidget.h"

#include "Components/TextBlock.h"
#include "Components/EditableTextBox.h"
#include "Components/Button.h"

void UGProjectLobbyWidget::UpdatePlayerCountText(int32 CurrentPlayers, int32 RequiredPlayers)
{
	if (PlayerCountText)
	{
		FString ContentString = FString::Printf(TEXT("Players %d / %d"), CurrentPlayers, RequiredPlayers);
		PlayerCountText->SetText(FText::FromString(ContentString));
	}
}