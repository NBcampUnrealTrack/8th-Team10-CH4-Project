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
