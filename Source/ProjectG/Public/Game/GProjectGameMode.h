// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GProjectGameMode.generated.h"

UCLASS()
class PROJECTG_API AGProjectGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AGProjectGameMode();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Match")
	int32 MatchDuration = 180;

private:
	FTimerHandle MatchTimerHandle;

	void StartMatchTimer();
	void TickMatchTimer();
	void HandleMatchTimerExpired();
};
