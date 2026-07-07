// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/WidgetController/GProjectOverlayWidgetController.h"

#include "Engine/World.h"
#include "Game/GProjectGameState.h"
#include "GameFramework/PlayerController.h"
#include "Player/GProjectPlayerState.h"

void UGProjectOverlayWidgetController::BindCallbacksToDependencies()
{

	Super::BindCallbacksToDependencies();

	if (!PlayerController)
	{
		return;
	}

	if (AGProjectGameState* GameState = PlayerController->GetWorld()->GetGameState<AGProjectGameState>())
	{
		GameState->OnPlayerListChanged.RemoveAll(this);

		GameState->OnMatchTimeChanged.AddUObject(this, &ThisClass::HandleMatchTimeChanged);
		GameState->OnPlayerListChanged.AddUObject(this, &ThisClass::HandlePlayerListChanged);
		GameState->OnChatMessageReceived.AddUObject(this, &ThisClass::HandleChatMessageReceived);
	}

	BindTeamCallbacks();
}

void UGProjectOverlayWidgetController::BroadcastInitialValues()
{
	Super::BroadcastInitialValues();

	if (!PlayerController)
	{
		return;
	}

	if (AGProjectGameState* GS = PlayerController->GetWorld()->GetGameState<AGProjectGameState>())
	{
		OnMatchTimeChanged.Broadcast(GS->GetRemainMatchTime());
	}
}

TArray<AGProjectPlayerState*> UGProjectOverlayWidgetController::GetOrderedPlayerStates() const
{
	TArray<AGProjectPlayerState*> OrderedPlayerStates;
	AGProjectPlayerState* LocalPlayerState = Cast<AGProjectPlayerState>(PlayerState);
	if (LocalPlayerState)
	{
		OrderedPlayerStates.Add(LocalPlayerState);
	}

	const UWorld* World = PlayerController ? PlayerController->GetWorld() : nullptr;
	const AGProjectGameState* GameState = World ? World->GetGameState<AGProjectGameState>() : nullptr;
	if (!GameState)
	{
		return OrderedPlayerStates;
	}

	TArray<AGProjectPlayerState*> OtherPlayerStates;
	for (APlayerState* CurrentPlayerState : GameState->PlayerArray)
	{
		AGProjectPlayerState* GProjectPlayerState = Cast<AGProjectPlayerState>(CurrentPlayerState);
		if (GProjectPlayerState && GProjectPlayerState != LocalPlayerState)
		{
			OtherPlayerStates.Add(GProjectPlayerState);
		}
	}

	OtherPlayerStates.Sort(
		[](const AGProjectPlayerState& Left, const AGProjectPlayerState& Right)
		{
			return Left.GetPlayerId() < Right.GetPlayerId();
		});
	OrderedPlayerStates.Append(OtherPlayerStates);

	return OrderedPlayerStates;
}

void UGProjectOverlayWidgetController::HandlePlayerListChanged()
{
	BindTeamCallbacks();
	
	OnPlayerListChanged.Broadcast();
}

void UGProjectOverlayWidgetController::HandleMatchTimeChanged(int32 RemainTime)
{
	OnMatchTimeChanged.Broadcast(RemainTime);
}

void UGProjectOverlayWidgetController::HandleChatMessageReceived(int32 SenderPlayerID, const FString& SenderName, const FString& Message)
{
	OnChatMessageReceived.Broadcast(
		SenderPlayerID,
		SenderName,
		Message
	);

}

void UGProjectOverlayWidgetController::BindTeamCallbacks()
{
	if (!PlayerController)
	{
		return;
	}

	AGProjectGameState* GS = PlayerController->GetWorld()->GetGameState<AGProjectGameState>();
	if (!GS)
	{
		return;
	}

	for (APlayerState* BasePlayerState : GS->PlayerArray)
	{
		AGProjectPlayerState* PS = Cast<AGProjectPlayerState>(BasePlayerState);
		if (!PS)
		{
			continue;
		}

		PS->OnTeamChanged.RemoveAll(this);
		PS->OnTeamChanged.AddUObject(this, &ThisClass::HandlePlayerTeamChanged);
	}
}

void UGProjectOverlayWidgetController::HandlePlayerTeamChanged(EGProjectTeam NewTeam)
{
	(void)NewTeam;

	OnPlayerListChanged.Broadcast();
}
