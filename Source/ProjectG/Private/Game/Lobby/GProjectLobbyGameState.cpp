// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/Lobby/GProjectLobbyGameState.h"
#include "Player/GProjectPlayerState.h"

bool AGProjectLobbyGameState::CanStartGame() const
{
	if (PlayerArray.Num() < 2)
	{
		return false;
	}

	for (APlayerState* PS : PlayerArray)
	{
		if (AGProjectPlayerState* GProjectPS = Cast<AGProjectPlayerState>(PS))
		{
			if (!GProjectPS->bIsHost && !GProjectPS->bIsReady)
			{
				return false;
			}
		}
	}

	return true;
}