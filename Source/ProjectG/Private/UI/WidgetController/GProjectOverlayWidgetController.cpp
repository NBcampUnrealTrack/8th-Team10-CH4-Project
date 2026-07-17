// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/WidgetController/GProjectOverlayWidgetController.h"

#include "Engine/World.h"
#include "Game/GProjectGameState.h"
#include "GameFramework/PlayerController.h"
#include "Player/GProjectPlayerState.h"
#include "UI/Widget/GProjectMatchHeaderWidget.h"

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
		GameState->OnTeamRoundWinsChanged.RemoveAll(this);

		GameState->OnMatchTimeChanged.AddUObject(this, &ThisClass::HandleMatchTimeChanged);
		GameState->OnPlayerListChanged.AddUObject(this, &ThisClass::HandlePlayerListChanged);
		GameState->OnChatMessageReceived.AddUObject(this, &ThisClass::HandleChatMessageReceived);
		GameState->OnRoundPhaseChanged.AddUObject(this, &ThisClass::HandleRoundPhaseChanged);
		GameState->OnTeamRoundWinsChanged.AddUObject(this, &ThisClass::HandleTeamRoundWinsChanged);
		GameState->OnKillFeedReceived.RemoveAll(this);
		GameState->OnKillFeedReceived.AddUObject(this, &ThisClass::HandleKillFeedReceived);

		GameState->OnRoundCountdownChanged.RemoveAll(this);
		GameState->OnRoundCountdownChanged.AddUObject(this, &ThisClass::HandleRoundCountdownChanged);

		GameState->OnRoundResultReceived.RemoveAll(this);
		GameState->OnRoundResultReceived.AddUObject(this, &ThisClass::HandleRoundResultReceived);
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

		OnRoundPhaseUIChanged.Broadcast(GS->GetRoundPhase(), GS->GetCurrentRound());

		OnTeamScoreUIChanged.Broadcast(GS->GetRedTeamRoundWins(), GS->GetBlueTeamRoundWins());

		if (GS->GetRoundPhase() == ERoundPhase::RoundResult)
		{
			const FGProjectRoundResultData& RoundResultData = GS->GetLastRoundResultData();

			if (RoundResultData.Result != ERoundResult::None)
			{
				OnRoundResultUIReceived.Broadcast(RoundResultData);
			}
		}
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

void UGProjectOverlayWidgetController::HandleRoundPhaseChanged(const ERoundPhase NewPhase)
{
	if (!PlayerController)
	{
		return;
	}

	const AGProjectGameState* GS =
		PlayerController
		->GetWorld()
		->GetGameState<AGProjectGameState>();

	if (!GS)
	{
		return;
	}

	OnRoundPhaseUIChanged.Broadcast(
		NewPhase,
		GS->GetCurrentRound()
	);

	if (NewPhase == ERoundPhase::RoundResult)
	{
		const FGProjectRoundResultData& RoundResultData = GS->GetLastRoundResultData();

		if (RoundResultData.Result != ERoundResult::None)
		{
			OnRoundResultUIReceived.Broadcast(RoundResultData);
		}
	}
}

void UGProjectOverlayWidgetController::HandleTeamRoundWinsChanged(int32 RedTeamWins, int32 BlueTeamWins)
{
	OnTeamScoreUIChanged.Broadcast(
		RedTeamWins,
		BlueTeamWins
	);
}

void UGProjectOverlayWidgetController::HandleKillFeedReceived(
	const int32 KillerPlayerId,
	const FString& KillerName,
	const int32 KillerColorIndex,
	const int32 VictimPlayerId,
	const FString& VictimName,
	const int32 VictimColorIndex)
{
	OnKillFeedReceived.Broadcast(
		KillerPlayerId,
		KillerName,
		KillerColorIndex,
		VictimPlayerId,
		VictimName,
		VictimColorIndex
	);
}

void UGProjectOverlayWidgetController::HandleRoundCountdownChanged(const int32 CountdownValue)
{
	OnRoundCountdownChanged.Broadcast(CountdownValue);
}

void UGProjectOverlayWidgetController::HandleRoundResultReceived(const FGProjectRoundResultData& RoundResultData)
{
	if (!PlayerController)
	{
		return;
	}

	const AGProjectGameState* GS =
		PlayerController
		->GetWorld()
		->GetGameState<AGProjectGameState>();

	if (!GS)
	{
		return;
	}

	if (GS->GetRoundPhase() != ERoundPhase::RoundResult)
	{
		return;
	}

	OnRoundResultUIReceived.Broadcast(RoundResultData);
}