// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/Lobby/GProjectSessionRowWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void UGProjectSessionRowWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &UGProjectSessionRowWidget::HandleJoinButtonClicked);
	}
}

void UGProjectSessionRowWidget::SetupSessionRow(int32 InSessionIndex, const FString& InRoomName, int32 CurrentPlayers, int32 MaxPlayers)
{
	SessionIndex = InSessionIndex;

	if (RoomNameText)
	{
		RoomNameText->SetText(FText::FromString(InRoomName));
	}

	if (PlayerCountText)
	{
		FString CountStr = FString::Printf(TEXT("%d / %d"), CurrentPlayers, MaxPlayers);
		PlayerCountText->SetText(FText::FromString(CountStr));
	}
}

void UGProjectSessionRowWidget::HandleJoinButtonClicked()
{
	if (OnSessionRowClicked.IsBound() && SessionIndex != INDEX_NONE)
	{
		OnSessionRowClicked.Broadcast(SessionIndex);
	}
}