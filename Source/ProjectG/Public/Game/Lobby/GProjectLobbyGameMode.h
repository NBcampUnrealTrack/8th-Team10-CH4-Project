// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GProjectLobbyGameMode.generated.h"

class AGProjectLobbyPlayerController;

UCLASS()
class PROJECTG_API AGProjectLobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	AGProjectLobbyGameMode();
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual void BeginPlay() override;

	int32 GetRequiredPlayers() const;
	bool CanStartGame() const;
	void StartGame();

private:
	void UpdatePlayerCountUI();
	void CheckAutoStart();


protected:
	UPROPERTY(EditDefaultsOnly, Category = "Lobby Settings")
	int32 RequiredPlayers = 2;

	UPROPERTY(EditDefaultsOnly, Category = "Lobby Settings")
	FString BattleMapPath = TEXT("/Game/Level/TestLevel");

private:
	bool bIsStartingGame = false;
};
