// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/GProjectGameMode.h"

#include "Character/GProjectCharacter.h"
#include "Game/GProjectGameState.h"
#include "Player/GProjectPlayerController.h"
#include "Player/GProjectPlayerState.h"
#include "UI/HUD/GProjectHUD.h"
#include "TimerManager.h"

AGProjectGameMode::AGProjectGameMode()
{
	DefaultPawnClass = AGProjectCharacter::StaticClass();
	PlayerControllerClass = AGProjectPlayerController::StaticClass();
	PlayerStateClass = AGProjectPlayerState::StaticClass();
	GameStateClass = AGProjectGameState::StaticClass();
	HUDClass = AGProjectHUD::StaticClass();
}

void AGProjectGameMode::BeginPlay()
{
	Super::BeginPlay();

	StartMatchTimer();
}

void AGProjectGameMode::StartMatchTimer()
{
	AGProjectGameState* GS = GetGameState<AGProjectGameState>();
	if (!GS)
	{
		return;
	}

	GS->SetRemainMatchTime(MatchDuration);

	GetWorldTimerManager().SetTimer(
		MatchTimerHandle,
		this,
		&ThisClass::TickMatchTimer,
		1.0f,
		true,
		1.0f
	);
}

void AGProjectGameMode::TickMatchTimer()
{
	AGProjectGameState* GS = GetGameState<AGProjectGameState>();
	if (!GS) 
	{
		return;
	}

	const int NewTime = GS->GetRemainMatchTime() - 1;
	GS->SetRemainMatchTime(NewTime);

	if (NewTime <= 0)
	{
		GetWorldTimerManager().ClearTimer(MatchTimerHandle);
		HandleMatchTimerExpired();
	}
}

void AGProjectGameMode::HandleMatchTimerExpired()
{
	// 추후 승패 판정, UI 표시 등 추가 예정
}
