// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/GProjectGameState.h"

#include "Net/UnrealNetwork.h"
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

void AGProjectGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGProjectGameState, RemainMatchTime);

	DOREPLIFETIME(AGProjectGameState, CurrentRound);

	DOREPLIFETIME(AGProjectGameState, RoundPhase);
}

void AGProjectGameState::SetRemainMatchTime(int32 Time)
{
	RemainMatchTime = FMath::Max(Time, 0);

	OnMatchTimeChanged.Broadcast(RemainMatchTime);
}

void AGProjectGameState::OnRep_RemainMatchTime()
{
	OnMatchTimeChanged.Broadcast(RemainMatchTime);
}

void AGProjectGameState::OnRep_CurrentRound()
{
	OnCurrentRoundChanged.Broadcast(CurrentRound);
}

void AGProjectGameState::OnRep_RoundPhase()
{
	OnRoundPhaseChanged.Broadcast(RoundPhase);
}

void AGProjectGameState::BroadcastChatMessage(int32 SenderPlayerID, const FString& SenderName, const FString& Message)
{
	if (!HasAuthority())
	{
		return;
	}

	MulticastReceiveChatMessage(SenderPlayerID, SenderName, Message);
}

void AGProjectGameState::SetCurrentRound(int32 NewRound)
{
	if (!HasAuthority() || CurrentRound == NewRound)
	{
		return;
	}

	CurrentRound = NewRound;
	OnCurrentRoundChanged.Broadcast(CurrentRound);
}

int32 AGProjectGameState::GetCurrentRound() const
{
	return CurrentRound;
}

void AGProjectGameState::SetRoundPhase(ERoundPhase NewPhase)
{
	if (!HasAuthority() || RoundPhase == NewPhase)
	{
		return;
	}

	RoundPhase = NewPhase;
	OnRoundPhaseChanged.Broadcast(RoundPhase);
}

ERoundPhase AGProjectGameState::GetRoundPhase() const
{
	return RoundPhase;
}

void AGProjectGameState::MulticastReceiveChatMessage_Implementation(int32 SenderPlayerID, const FString& SenderName, const FString& Message)
{
	OnChatMessageReceived.Broadcast(SenderPlayerID, SenderName, Message);
}
