// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/Lobby/GProjectLobbyGameMode.h"
#include "Player/Lobby/GProjectLobbyPlayerController.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystem/GProjectSessionSubsystem.h"
#include "OnlineSubsystem.h"

AGProjectLobbyGameMode::AGProjectLobbyGameMode()
{
	bUseSeamlessTravel = true;
}

void AGProjectLobbyGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	int32 CustomMaxPlayers = UGameplayStatics::GetIntOption(Options, TEXT("MaxPlayersCustom"), 0);
	if (CustomMaxPlayers >= 2)
	{
		RequiredPlayers = CustomMaxPlayers;
	}

	FString CustomMapPath = UGameplayStatics::ParseOption(Options, TEXT("BATTLE_MAP_PATH"));
	if (!CustomMapPath.IsEmpty())
	{
		BattleMapPath = CustomMapPath;
	}
}

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

void AGProjectLobbyGameMode::BeginPlay()
{
	Super::BeginPlay();

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

	UWorld* World = GetWorld();
	if (!World) return;

	int32 CurrentPlayers = 0;
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		if (It->Get())
		{
			CurrentPlayers++;
		}
	}
	if (CurrentPlayers >= RequiredPlayers)
	{
		StartGame();
	}
}

void AGProjectLobbyGameMode::StartGame()
{
	UE_LOG(LogTemp, Warning, TEXT("StartGame Called"));

	if (bIsStartingGame)
	{
		UE_LOG(LogTemp, Warning, TEXT("Blocked: bIsStartingGame true"));
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("Blocked: World null"));
		return;
	}

	FString FinalBattleMapPath = BattleMapPath;

	UE_LOG(LogTemp, Warning, TEXT("BattleMapPath: %s"), *BattleMapPath);
	UE_LOG(LogTemp, Warning, TEXT("FinalBattleMapPath: %s"), *FinalBattleMapPath);

	if (FinalBattleMapPath.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("Blocked: FinalBattleMapPath empty"));
		return;
	}

	bIsStartingGame = true;

	FString TravelURL = FString::Printf(TEXT("%s?listen"), *FinalBattleMapPath);
	UE_LOG(LogTemp, Warning, TEXT("ServerTravel URL: %s"), *TravelURL);

	World->ServerTravel(TravelURL);
}

int32 AGProjectLobbyGameMode::GetRequiredPlayers() const
{
	return RequiredPlayers;
}

