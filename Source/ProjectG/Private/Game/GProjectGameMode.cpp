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
#include "Item/SpawnItem/SpawnBase.h"


#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Item/GItemHolderComponent.h"
#include "Item/GItemPickup.h"
#include "Kismet/GameplayStatics.h"

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

	AGProjectPlayerState* PS = NewPlayer->GetPlayerState<AGProjectPlayerState>();
	if (!PS)
	{
		return;
	}

	AssignPlayerColor(PS);

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

void AGProjectGameMode::NotifyPlayerDied(AGProjectPlayerState* DeadPlayerState)
{
	if (!HasAuthority() || !DeadPlayerState)
	{
		return;
	}

	AGProjectGameState* GS = GetGameState<AGProjectGameState>();
	if (!GS)
	{
		return;
	}

	if (!IsMatchInProgress() || GS->GetRoundPhase() != ERoundPhase::Playing)
	{
		return;
	}

	const EGProjectTeam DeadTeam = DeadPlayerState->GetTeam();
	if (DeadTeam == EGProjectTeam::None)
	{
		return;
	}
	APlayerController* DeadPC = Cast<APlayerController>(DeadPlayerState->GetOwner());
	if (DeadPC)
	{
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			APlayerController* OtherPC = It->Get();
			if (!OtherPC || OtherPC == DeadPC) continue;
			
			AGProjectCharacter* LivingCharacter = Cast<AGProjectCharacter>(OtherPC->GetPawn());
          
			if (LivingCharacter && !LivingCharacter->IsDead())
			{
				DeadPC->SetViewTargetWithBlend(LivingCharacter, 0.5f);
				break; 
			}
		}
	}

	if (!IsTeamEliminated(DeadTeam))
	{
		UE_LOG(
		   LogTemp,
		   Warning,
		   TEXT("Player Died, But team still alive | PlayerID = %d"),
		   DeadPlayerState->GetPlayerId()
		);
		return;
	}

	const EGProjectTeam Winner = DeadTeam == EGProjectTeam::Red ? EGProjectTeam::Blue : EGProjectTeam::Red;

	GS->AddTeamRoundWin(Winner);

	FinishRound();
}

void AGProjectGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpawnBase::StaticClass(), (TArray<AActor*>&)SpawnZones);
	
	if (ItemPool.Num() > 0 && SpawnZones.Num() > 0)
	{
		for (int32 i = 0; i < 5; ++i)
		{
			SpawnRandomItem();
		}
		
		GetWorldTimerManager().SetTimer(
		   ItemSpawnTimerHandle,
		   this,
		   &AGProjectGameMode::SpawnRandomItem,
		   10.0f,
		   true
		);
	}
}

void AGProjectGameMode::SpawnRandomItem()
{
	if (CurrentSpawnedItems >= MaxSpawnedItems) return;
	
	if (ItemPool.Num() == 0 || SpawnZones.Num() == 0) return;
	
	float TotalWeight = 0.0f;
	for (UItemSpawnDataAsset* Asset : ItemPool)
	{
		if (Asset)
		{
			TotalWeight += Asset->SpawnWeight;
		}
	}

	if (TotalWeight <= 0.0f) return;
    
	float DiceRoll = FMath::FRandRange(0.0f, TotalWeight);
    
	UItemSpawnDataAsset* ChosenAsset = nullptr;
	for (UItemSpawnDataAsset* Asset : ItemPool)
	{
		if (!Asset) continue;

		DiceRoll -= Asset->SpawnWeight;
		if (DiceRoll <= 0.0f)
		{
			ChosenAsset = Asset;
			break;
		}
	}
    
	if (!ChosenAsset)
	{
		ChosenAsset = ItemPool.Last();
	}
	
	if (ChosenAsset && ChosenAsset->ItemClass)
	{
		int32 RandomZoneIndex = FMath::RandRange(0, SpawnZones.Num() - 1);
		ASpawnBase* TargetZone = SpawnZones[RandomZoneIndex];

		if (TargetZone)
		{
			TargetZone->SpawnItem(ChosenAsset->ItemClass);
			CurrentSpawnedItems++;
		}
	}
}

void AGProjectGameMode::DecreaseSpawnedItemCount()
{
	CurrentSpawnedItems = FMath::Max(0, CurrentSpawnedItems - 1);
}

void AGProjectGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	AGProjectGameState* GS = GetGameState<AGProjectGameState>();

	if (!GS)
	{
		return;
	}

	GS->ResetTeamRoundWins();
	GS->SetCurrentRound(1);

	StartRound();
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

	GetWorldTimerManager().ClearTimer(
		RoundCountdownTimerHandle
	);
}

void AGProjectGameMode::TickRoundCountdown()
{
	if (!IsMatchInProgress())
	{
		GetWorldTimerManager().ClearTimer(RoundCountdownTimerHandle);

		return;
	}

	AGProjectGameState* GS = GetGameState<AGProjectGameState>();

	if (!GS)
	{
		GetWorldTimerManager().ClearTimer(RoundCountdownTimerHandle);

		return;
	}

	if (GS->GetRoundPhase() != ERoundPhase::Countdown)
	{
		GetWorldTimerManager().ClearTimer(RoundCountdownTimerHandle);

		return;
	}

	--CurrentRoundCountdownValue;

	if (CurrentRoundCountdownValue > 0)
	{
		GS->BroadcastRoundCountdown(CurrentRoundCountdownValue);

		return;
	}

	GetWorldTimerManager().ClearTimer(RoundCountdownTimerHandle);

	GS->BroadcastRoundCountdown(0);

	BeginRoundFight();
}

void AGProjectGameMode::BeginRoundFight()
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

	GetWorldTimerManager().ClearTimer(MatchTimerHandle);

	GetWorldTimerManager().SetTimer(
		MatchTimerHandle,
		this,
		&ThisClass::TickMatchTimer,
		1.0f,
		true
	);
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

	GetWorldTimerManager().ClearTimer(MatchTimerHandle);

	GetWorldTimerManager().ClearTimer(RoundCountdownTimerHandle);

	GS->SetRemainMatchTime(RoundDuration);

	GS->SetRoundPhase(ERoundPhase::Countdown);

	CurrentRoundCountdownValue = FMath::Max(RoundCountdownStartValue, 1);

	GS->BroadcastRoundCountdown(CurrentRoundCountdownValue);

	GetWorldTimerManager().SetTimer(
		RoundCountdownTimerHandle,
		this,
		&ThisClass::TickRoundCountdown,
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

	GetWorldTimerManager().ClearTimer(MatchTimerHandle);

	if (HasTeamWonMatch())
	{
		GS->SetRoundPhase(ERoundPhase::Finished);

		GetWorldTimerManager().SetTimer(
			RoundTransitionTimerHandle, 
			this, 
			&ThisClass::FinishMatchAfterDelay, 
			RoundTransitionDuration, 
			false
		);

		return;
	}

	GS->SetRoundPhase(ERoundPhase::Intermission);

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

	int32 ControllerCount = 0;

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		++ControllerCount;

		APlayerController* PC = It->Get();

		if (!PC)
		{
			continue;
		}

		APawn* Pawn = PC->GetPawn();
		if (!Pawn)
		{
			continue;
		}

		AGProjectCharacter* Character =	Cast<AGProjectCharacter>(Pawn);

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

		if (UGProjectItemHolderComponent* ItemHolder =
			Character->GetItemHolderComponent())
		{
			ItemHolder->DropHeldItem();
		}

		if (UGItemHolderComponent* ConsumableHolder =
			Character->FindComponentByClass<UGItemHolderComponent>())
		{
			ConsumableHolder->ClearHeldItem();
		}
	}

	int32 ResetItemCount = 0;

	for (TActorIterator<AGProjectItemActorBase> It(World); It; ++It)
	{
		AGProjectItemActorBase* Item = *It;

		if (!Item)
		{
			continue;
		}

		Item->ResetToSpawnTransform();
		++ResetItemCount;
	}

	for (TActorIterator<AGItemPickup> It(World); It; ++It)
	{
		AGItemPickup* Pickup = *It;

		if (!Pickup)
		{
			continue;
		}

		Pickup->ResetForNewRound();
	}

	for (int32 Index = 0; Index < Characters.Num(); ++Index)
	{
		APlayerController* PC = PlayerControllers.IsValidIndex(Index) ? PlayerControllers[Index] : nullptr;

		AGProjectCharacter* Character = Characters.IsValidIndex(Index) ? Characters[Index] : nullptr;

		if (!PC || !Character)
		{
			continue;
		}

		AActor* PlayerStart = FindPlayerStart(PC);

		if (!PlayerStart)
		{
			continue;
		}

		Character->ResetForNewRound(
			PlayerStart->GetActorTransform()
		);

		PC->SetControlRotation(
			PlayerStart->GetActorRotation()
		);
		
		PC->SetViewTarget(Character);
	}

}

bool AGProjectGameMode::IsTeamEliminated(EGProjectTeam Team) const
{
	if (!GetWorld() || Team == EGProjectTeam::None)
	{
		return false;
	}

	bool bFoundTeamMember = false;

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC)
		{
			continue;
		}

		AGProjectPlayerState* PS = PC->GetPlayerState<AGProjectPlayerState>();
		if (!PS || PS->GetTeam() != Team)
		{
			continue;
		}

		bFoundTeamMember = true;

		const AGProjectCharacter* Character = Cast<AGProjectCharacter>(PC->GetPawn());
		if (Character && !Character->IsDead())
		{
			return false;
		}
	}

	return bFoundTeamMember;
}

bool AGProjectGameMode::HasTeamWonMatch() const
{
	const AGProjectGameState* GS = GetGameState<AGProjectGameState>();
	if (!GS)
	{
		return false;
	}

	return (GS->GetRedTeamRoundWins() >= RoundsToWin || GS->GetBlueTeamRoundWins() >= RoundsToWin);
}

void AGProjectGameMode::AssignPlayerColor(AGProjectPlayerState* PS)
{
	if (!PS)
	{
		return;
	}

	TSet<int32> UsedColorIndices;

	AGProjectGameState* GS = GetGameState<AGProjectGameState>();
	
	if (GS)
	{
		for (APlayerState* BasePS : GS->PlayerArray)
		{
			const AGProjectPlayerState* OtherPS = Cast<AGProjectPlayerState>(BasePS);

			if (!OtherPS || OtherPS == PS)
			{
				continue;
			}

			const int32 ColorIndex = OtherPS->GetPlayerColorIndex();

			if (ColorIndex != INDEX_NONE)
			{
				UsedColorIndices.Add(ColorIndex);
			}
		}
	}

	for (int32 Index = 0; Index < 4; ++Index)
	{
		if (!UsedColorIndices.Contains(Index))
		{
			PS->SetPlayerColorIndex(Index);
			return;
		}
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
