// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/GProjectGameState.h"

void AGProjectGameState::AddPlayerState(APlayerState* InPlayerState)
{
	const int32 PreviousPlayerCount = PlayerArray.Num();
	Super::AddPlayerState(InPlayerState);

	if (PlayerArray.Num() != PreviousPlayerCount)
	{
		OnPlayerListChanged.Broadcast();
	}
}

void AGProjectGameState::RemovePlayerState(APlayerState* InPlayerState)
{
	const int32 PreviousPlayerCount = PlayerArray.Num();
	Super::RemovePlayerState(InPlayerState);

	if (PlayerArray.Num() != PreviousPlayerCount)
	{
		OnPlayerListChanged.Broadcast();
	}
}

void AGProjectGameState::BroadcastChatMessage(const FString& SenderName, const FString& Message)
{
	if (!HasAuthority())
	{
		return;
	}

	MulticastReceiveChatMessage(SenderName, Message);
}

void AGProjectGameState::MulticastReceiveChatMessage_Implementation(const FString& SenderName, const FString& Message)
{
	OnChatMessageReceived.Broadcast(SenderName, Message);
}

