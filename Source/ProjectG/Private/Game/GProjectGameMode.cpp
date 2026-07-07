// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/GProjectGameMode.h"

#include "Character/GProjectCharacter.h"
#include "Game/GProjectGameState.h"
#include "Player/GProjectPlayerController.h"
#include "Player/GProjectPlayerState.h"
#include "UI/HUD/GProjectHUD.h"
#include "TimerManager.h"
#include "EngineUtils.h"
#include "Item/GProjectItemActorBase.h"
#include "Item/GProjectItemHolderComponent.h"

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

	AssignTeam(NewPlayer);

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

	TArray<APlayerController*> PlayerControllers;
	TArray<AGProjectCharacter*> Characters;

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

		PlayerControllers.Add(PC);
		Characters.Add(Character);
	}

	for (AGProjectCharacter* Character : Characters)
	{
		if (!Character)
		{
			continue;
		}

		UGProjectItemHolderComponent* ItemHolder = Character->GetItemHolderComponent();

		if (ItemHolder)
		{
			ItemHolder->DropHeldItem();
		}
	}

	for (TActorIterator<AGProjectItemActorBase> It(World); It; ++It)
	{
		AGProjectItemActorBase* Item = *It;

		if (Item)
		{
			Item->ResetToSpawnTransform();
		}
	}

	for (int32 Index = 0; Index < Characters.Num(); ++Index)
	{
		APlayerController* PC = PlayerControllers[Index];
		AGProjectCharacter* Character = Characters[Index];

		if (!PC || !Character)
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

void AGProjectGameMode::AssignTeam(APlayerController* NewPlayer)
{
	if (!HasAuthority() || !NewPlayer)
	{
		return;
	}

	AGProjectPlayerState* NewPlayerState = NewPlayer->GetPlayerState<AGProjectPlayerState>();

	AGProjectGameState* GS = GetGameState<AGProjectGameState>();

	if (!NewPlayerState || !GS)
	{
		return;
	}

	if (NewPlayerState->GetTeam() != EGProjectTeam::None)
	{
		return;
	}

	int32 RedTeamCount = 0;
	int32 BlueTeamCount = 0;

	for (APlayerState* BasePlayerState : GS->PlayerArray)
	{
		const AGProjectPlayerState* PS = Cast<AGProjectPlayerState>(BasePlayerState);

		if (!PS)
		{
			continue;
		}

		switch (PS->GetTeam())
		{
		case EGProjectTeam::Red:
			++RedTeamCount;
			break;

		case EGProjectTeam::Blue:
			++BlueTeamCount;
			break;

		default:
			break;
		}
	}

	const EGProjectTeam NewTeam = RedTeamCount <= BlueTeamCount ? EGProjectTeam::Red : EGProjectTeam::Blue;

	NewPlayerState->SetTeam(NewTeam);

	const TCHAR* TeamName = NewTeam == EGProjectTeam::Red ? TEXT("Red") : TEXT("Blue");
}
