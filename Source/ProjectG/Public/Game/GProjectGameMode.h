// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "TimerManager.h"
#include "GProjectGameMode.generated.h"


class APlayerController;
class AGProjectPlayerState;

enum class EGProjectTeam : uint8;

UCLASS()
class PROJECTG_API AGProjectGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	AGProjectGameMode();

	virtual void PostLogin(APlayerController* NewPlayer) override;

	void NotifyPlayerDied(AGProjectPlayerState* DeadPlayerState);

protected:
	virtual void BeginPlay() override;

	virtual void HandleMatchHasStarted() override;
	virtual void HandleMatchHasEnded() override;

	void StartRound();
	void FinishRound();
	void StartNextRound();
	void TickMatchTimer();
	void FinishMatchAfterDelay();

	void ResetPlayersForNextRound();

	bool IsTeamEliminated(EGProjectTeam Team)const;

	bool HasTeamWonMatch() const;

	UPROPERTY(EditDefaultsOnly, Category = "Match")
	int32 RequiredPlayers = 2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Match")
	int32 RoundDuration = 180;

	UPROPERTY(EditDefaultsOnly, Category = "Round")
	int32 RoundsToWin = 2;

	UPROPERTY(EditDefaultsOnly, Category = "Round")
	float RoundTransitionDuration = 3.0f;

private:
	FTimerHandle MatchTimerHandle;
	FTimerHandle RoundTransitionTimerHandle;

	void AssignTeam(APlayerController* NewPlayer);
};
