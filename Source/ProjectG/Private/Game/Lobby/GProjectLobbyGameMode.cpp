// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/Lobby/GProjectLobbyGameMode.h"
#include "Player/Lobby/GProjectLobbyPlayerController.h"
#include "GameFramework/GameStateBase.h"

void AGProjectLobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	UpdatePlayerCountUI();

	CheckAutoStart();
}

void AGProjectLobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	UpdatePlayerCountUI();
}

void AGProjectLobbyGameMode::UpdatePlayerCountUI()
{
	if (!GameState) return;

	int32 CurrentPlayers = GameState->PlayerArray.Num();
	UWorld* World = GetWorld();
	if (!World) return;

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC)
		{
			AGProjectLobbyPlayerController* LobbyPC = Cast<AGProjectLobbyPlayerController>(PC);
			if (LobbyPC)
			{
				LobbyPC->ClientUpdatePlayerCount(CurrentPlayers, RequiredPlayers);
			}
		}
	}
}

void AGProjectLobbyGameMode::CheckAutoStart()
{
	if (!GameState) return;
	if (bIsStartingGame) return;

	int32 CurrentPlayers = GameState->PlayerArray.Num();
	if (CurrentPlayers >= RequiredPlayers)
	{
		StartGame();
	}
}

void AGProjectLobbyGameMode::StartGame()
{
	if (bIsStartingGame) return;

	UWorld* World = GetWorld();
	if (!World || BattleMapPath.IsEmpty())
	{
		return;
	}

	bIsStartingGame = true;

	FString TravelURL = BattleMapPath + TEXT("?listen");
	World->ServerTravel(TravelURL);
}