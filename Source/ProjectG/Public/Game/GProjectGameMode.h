// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "TimerManager.h"
#include "GProjectGameMode.generated.h"


class APlayerController;

UCLASS()
class PROJECTG_API AGProjectGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	AGProjectGameMode();

	virtual void PostLogin(APlayerController* NewPlayer) override;

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

	UPROPERTY(EditDefaultsOnly, Category = "Match")
	int32 RequiredPlayers = 2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Match")
	int32 RoundDuration = 180;

	UPROPERTY(EditDefaultsOnly, Category = "Round")
	int32 MaxRounds = 3;

	UPROPERTY(EditDefaultsOnly, Category = "Round")
	float RoundTransitionDuration = 3.0f;

private:
	FTimerHandle MatchTimerHandle;
	FTimerHandle RoundTransitionTimerHandle;
};
