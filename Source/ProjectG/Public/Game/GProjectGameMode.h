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

	UPROPERTY(EditDefaultsOnly, Category = "Match")
	int32 RequiredPlayers = 2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Match")
	int32 RoundDuration = 180;

	UPROPERTY(EditDefaultsOnly, Category = "Round")
	int32 RoundsToWin = 2;

	UPROPERTY(EditDefaultsOnly, Category = "Round")
	float RoundTransitionDuration = 3.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning")
	TArray<UItemSpawnDataAsset*> ItemPool;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning|Limit")
	int32 MaxSpawnedItems = 10;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawning|Limit")
	int32 CurrentSpawnedItems = 0;
	
	FTimerHandle ItemSpawnTimerHandle;
	
	UFUNCTION(BlueprintCallable, Category = "Spawning")
	void SpawnRandomItem();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning")
	TArray<ASpawnBase*> SpawnZones;

private:
	FTimerHandle MatchTimerHandle;
	FTimerHandle RoundTransitionTimerHandle;

	void AssignTeam(APlayerController* NewPlayer);
};
