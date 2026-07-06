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
	bDelayedStart = true;

	DefaultPawnClass = AGProjectCharacter::StaticClass();
	PlayerControllerClass = AGProjectPlayerController::StaticClass();
	PlayerStateClass = AGProjectPlayerState::StaticClass();
	GameStateClass = AGProjectGameState::StaticClass();
	HUDClass = AGProjectHUD::StaticClass();
}

void AGProjectGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (HasMatchStarted())
	{
		return;
	}

	if (GetNumPlayers() < RequiredPlayers)
	{
		return;
	}

	StartMatch();
}

void AGProjectGameMode::BeginPlay()
{
	Super::BeginPlay();
}

void AGProjectGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	AGProjectGameState* GS = GetGameState<AGProjectGameState>();

	if (!GS)
	{
		return;
	}

	GS->SetCurrentRound(1);

	StartRound();

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("Match Started / State: %s"),
		*GetMatchState().ToString()
	);
}

void AGProjectGameMode::HandleMatchHasEnded()
{
	Super::HandleMatchHasEnded();

	GetWorldTimerManager().ClearTimer(
		MatchTimerHandle
	);

	GetWorldTimerManager().ClearTimer(
		RoundTransitionTimerHandle
	);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("Match Ended / State: %s"),
		*GetMatchState().ToString()
	);

	// 최종 승자, 결과 UI 로비 이동 등.....
}

void AGProjectGameMode::StartRound()
{
	if (!IsMatchInProgress())
	{
		return;
	}

	AGProjectGameState* GS = GetGameState<AGProjectGameState>();
	if (!GS)
	{
		return;
	}

	GS->SetRoundPhase(ERoundPhase::Playing);

	GS->SetRemainMatchTime(RoundDuration);

	GetWorldTimerManager().ClearTimer(
		MatchTimerHandle
	);

	GetWorldTimerManager().SetTimer(
		MatchTimerHandle,
		this,
		&ThisClass::TickMatchTimer,
		1.0f,
		true
	);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("Round %d Start"),
		GS->GetCurrentRound()
	);
}

void AGProjectGameMode::FinishRound()
{
	AGProjectGameState* GS = GetGameState<AGProjectGameState>();

	if (!GS)
	{
		return;
	}

	if (GS->GetRoundPhase() != ERoundPhase::Playing)
	{
		return;
	}

	GS->SetRoundPhase(ERoundPhase::Intermission);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("Round %d Finished"),
		GS->GetCurrentRound()
	);

	const bool bLastRound = GS->GetCurrentRound() >= MaxRounds;

	if (bLastRound)
	{
		GetWorldTimerManager().SetTimer(
			RoundTransitionTimerHandle,
			this,
			&ThisClass::FinishMatchAfterDelay,
			RoundTransitionDuration,
			false
		);

		return;
	}

	GetWorldTimerManager().SetTimer(
		RoundTransitionTimerHandle,
		this,
		&ThisClass::StartNextRound,
		RoundTransitionDuration,
		false
	);
}

void AGProjectGameMode::StartNextRound()
{
	if (!IsMatchInProgress())
	{
		return;
	}

	AGProjectGameState* GS = GetGameState<AGProjectGameState>();
	if (!GS)
	{
		return;
	}

	const int32 NextRound = GS->GetCurrentRound() + 1;

	GS->SetCurrentRound(NextRound);

	ResetPlayersForNextRound();

	StartRound();
}

void AGProjectGameMode::TickMatchTimer()
{
	AGProjectGameState* GS = GetGameState<AGProjectGameState>();
	if (!GS) 
	{
		GetWorldTimerManager().ClearTimer(
			MatchTimerHandle
		);

		return;
	}

	if (!IsMatchInProgress() || GS->GetRoundPhase() != ERoundPhase::Playing)
	{
		GetWorldTimerManager().ClearTimer(
			MatchTimerHandle
		);
		
		return;
	}

	const int32 NewRemainTime = FMath::Max(GS->GetRemainMatchTime() - 1, 0);

	GS->SetRemainMatchTime(NewRemainTime);

	if (NewRemainTime >  0)
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(
		MatchTimerHandle
	);

	FinishRound();
}

void AGProjectGameMode::FinishMatchAfterDelay()
{
	if (!IsMatchInProgress())
	{
		return;
	}

	EndMatch();
}

void AGProjectGameMode::ResetPlayersForNextRound()
{
	if (!HasAuthority())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();

		if (!PC)
		{
			continue;
		}

		AGProjectCharacter* Character = Cast<AGProjectCharacter>(PC->GetPawn());
		if (!Character)
		{
			continue;
		}

		AActor* PlayerStart = FindPlayerStart(PC);
		if (!PlayerStart)
		{
			continue;
		}

		Character->ResetForNewRound(PlayerStart->GetActorTransform());

		PC->SetControlRotation(PlayerStart->GetActorRotation());
	}
}
