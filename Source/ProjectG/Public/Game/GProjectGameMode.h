// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "TimerManager.h"
#include "Item/ItemSpawnDataAsset.h"
#include "GProjectGameMode.generated.h"

class ASpawnBase;
class APlayerController;
class AGProjectPlayerState;

enum class EGProjectTeam : uint8;
enum class ERoundResult : uint8;
enum class ERoundEndReason : uint8;

UCLASS()
class PROJECTG_API AGProjectGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	AGProjectGameMode();

	virtual void PostLogin(APlayerController* NewPlayer) override;

	void NotifyPlayerDied(AGProjectPlayerState* DeadPlayerState);
	
	UFUNCTION(BlueprintCallable, Category = "Spawning|Limit")
	void DecreaseSpawnedItemCount();

protected:
	virtual void BeginPlay() override;
	virtual void HandleSeamlessTravelPlayer(AController*& Controller) override;

	virtual void HandleMatchHasStarted() override;
	virtual void HandleMatchHasEnded() override;
	
	void StartRound();
	void FinishRound();
	void StartNextRound();
	void TickMatchTimer();
	void FinishMatchAfterDelay();
	void ResetPlayersForNextRound();
	
	void ClearPreviousRoundItems();
	
	bool IsTeamEliminated(EGProjectTeam Team)const;

	bool HasTeamWonMatch() const;

	void AssignPlayerColor(AGProjectPlayerState* PS);
	
	void SpectateOtherPlayer(class AGProjectPlayerState* DeadPlayerState);

	void TickRoundCountdown();
	void BeginRoundFight();

	void FinishRoundByHealth(ERoundEndReason Reason);

	void FinishRoundWithResult(
		ERoundResult Result,
		ERoundEndReason Reason,
		float RedTeamTotalHP,
		float BlueTeamTotalHP
	);

	void ContinueAfterRoundResult();

	float CalculateTeamTotalHealth(EGProjectTeam Team) const;

	ERoundResult DetermineResultFromHealth(float RedTeamTotalHP, float BlueTeamTotalHP) const;

	UPROPERTY(EditDefaultsOnly, Category = "Match")
	int32 RequiredPlayers = 2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Match")
	int32 RoundDuration = 180;

	UPROPERTY(EditDefaultsOnly, Category = "Round")
	int32 RoundsToWin = 2;

	UPROPERTY(EditDefaultsOnly, Category = "Round")
	float RoundTransitionDuration = 3.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Round")
	float RoundResultDisplayDuration = 2.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning")
	TArray<UItemSpawnDataAsset*> ItemPool;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning|Limit")
	int32 MaxSpawnedItems = 10;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawning|Limit")
	int32 CurrentSpawnedItems = 0;
	
	FTimerHandle ItemSpawnTimerHandle;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning|Balance")
	int32 StartItemSpawnCount = 5;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning|Balance", meta = (AllowPrivateAccess = "true"))
	float ItemSpawnInterval = 10.0f;
	
	UFUNCTION(BlueprintCallable, Category = "Spawning|Balance")
	void SpawnRandomItem();
	
	UPROPERTY()
	TArray<ASpawnBase*> SpawnZones;

	UPROPERTY(EditDefaultsOnly, Category = "Round")
	int32 RoundCountdownStartValue = 3;

private:
	void AssignTeam(APlayerController* NewPlayer);

	FTimerHandle MatchTimerHandle;
	FTimerHandle RoundTransitionTimerHandle;
	FTimerHandle RoundCountdownTimerHandle;
	FTimerHandle RoundResultTimerHandle;

	int CurrentRoundCountdownValue = 0;
};
