// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/WidgetController/GProjectOverlayWidgetController.h"

#include "Engine/World.h"
#include "Game/GProjectGameState.h"
#include "GameFramework/PlayerController.h"
#include "Player/GProjectPlayerState.h"

void UGProjectOverlayWidgetController::BindCallbacksToDependencies()
{
	if (!PlayerController)
	{
		return;
	}

	if (AGProjectGameState* GameState = PlayerController->GetWorld()->GetGameState<AGProjectGameState>())
	{
		GameState->OnPlayerListChanged.AddUObject(this, &ThisClass::HandlePlayerListChanged);
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
	OnPlayerListChanged.Broadcast();
}
