// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/GProjectGameState.h"

#include "Net/UnrealNetwork.h"
#include "Player/GProjectPlayerState.h"

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

	DOREPLIFETIME(AGProjectGameState, RedTeamRoundWins);

	DOREPLIFETIME(AGProjectGameState, BlueTeamRoundWins);
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

void AGProjectGameState::OnRep_TeamRoundWins()
{
	OnTeamRoundWinsChanged.Broadcast(RedTeamRoundWins, BlueTeamRoundWins);
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

void AGProjectGameState::AddTeamRoundWin(EGProjectTeam Winner)
{
	if (!HasAuthority())
	{
		return;
	}

	switch (Winner)
	{
	case EGProjectTeam::Red:
		++RedTeamRoundWins;
		break;

	case EGProjectTeam::Blue:
		++BlueTeamRoundWins;
		break;

	default:
		return;
	}

	OnTeamRoundWinsChanged.Broadcast(RedTeamRoundWins, BlueTeamRoundWins);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("Team Score | Red: %d, Blue: %d"),
		RedTeamRoundWins,
		BlueTeamRoundWins
	);
}

void AGProjectGameState::ResetTeamRoundWins()
{
	if (!HasAuthority())
	{
		return;
	}

	RedTeamRoundWins = 0;
	BlueTeamRoundWins = 0;

	OnTeamRoundWinsChanged.Broadcast(RedTeamRoundWins, BlueTeamRoundWins);
}

void AGProjectGameState::MulticastReceiveChatMessage_Implementation(int32 SenderPlayerID, const FString& SenderName, const FString& Message)
{
	OnChatMessageReceived.Broadcast(SenderPlayerID, SenderName, Message);
}
