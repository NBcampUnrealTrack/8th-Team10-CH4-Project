// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/GProjectGameMode.h"

#include "Actor/GProjectTeamPlayerStart.h"
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
#include "AbilitySystem/GProjectAttributeSet.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Actor/AGProjectCageActor.h"

AGProjectGameMode::AGProjectGameMode()
{
	bDelayedStart = true;
	bUseSeamlessTravel = true;

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

void AGProjectGameMode::Logout(AController* Exiting)
{
	AssignedTeamPlayerStarts.Remove(Exiting);

	Super::Logout(Exiting);
}

AActor* AGProjectGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	if (!Player)
	{
		return Super::ChoosePlayerStart_Implementation(Player);
	}

	const AGProjectPlayerState* PlayerState = Player->GetPlayerState<AGProjectPlayerState>();
	if (!PlayerState || PlayerState->GetTeam() == EGProjectTeam::None)
	{
		return Super::ChoosePlayerStart_Implementation(Player);
	}

	if (const TWeakObjectPtr<AGProjectTeamPlayerStart>* ExistingAssignment = AssignedTeamPlayerStarts.Find(Player))
	{
		AGProjectTeamPlayerStart* ExistingStart = ExistingAssignment->Get();
		if (IsValid(ExistingStart) && ExistingStart->GetStartTeam() == PlayerState->GetTeam())
		{
			return ExistingStart;
		}

		AssignedTeamPlayerStarts.Remove(Player);
	}

	TSet<AGProjectTeamPlayerStart*> UsedStarts;
	for (auto It = AssignedTeamPlayerStarts.CreateIterator(); It; ++It)
	{
		AController* AssignedController = It.Key().Get();
		AGProjectTeamPlayerStart* AssignedStart = It.Value().Get();
		if (!IsValid(AssignedController) || !IsValid(AssignedStart))
		{
			It.RemoveCurrent();
			continue;
		}

		UsedStarts.Add(AssignedStart);
	}

	TArray<AGProjectTeamPlayerStart*> Candidates;
	for (TActorIterator<AGProjectTeamPlayerStart> It(GetWorld()); It; ++It)
	{
		AGProjectTeamPlayerStart* TeamStart = *It;
		if (IsValid(TeamStart)
			&& TeamStart->GetStartTeam() == PlayerState->GetTeam()
			&& !UsedStarts.Contains(TeamStart))
		{
			Candidates.Add(TeamStart);
		}
	}

	Candidates.Sort([](const AGProjectTeamPlayerStart& Left, const AGProjectTeamPlayerStart& Right)
	{
		if (Left.GetSlotIndex() != Right.GetSlotIndex())
		{
			return Left.GetSlotIndex() < Right.GetSlotIndex();
		}

		return Left.GetFName().LexicalLess(Right.GetFName());
	});

	if (Candidates.IsEmpty())
	{
		return Super::ChoosePlayerStart_Implementation(Player);
	}

	AGProjectTeamPlayerStart* ChosenStart = Candidates[0];
	AssignedTeamPlayerStarts.Add(Player, ChosenStart);
	return ChosenStart;
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
	OpenCagesForTeam(DeadTeam);
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

	const EGProjectTeam Winner = 
		DeadTeam == EGProjectTeam::Red 
		? EGProjectTeam::Blue : EGProjectTeam::Red;

	const ERoundResult RoundResult =
		Winner == EGProjectTeam::Red
		? ERoundResult::RedWin : ERoundResult::BlueWin;

	const float RedTeamTotalHP = CalculateTeamTotalHealth(EGProjectTeam::Red);
	const float BlueTeamTotalHP = CalculateTeamTotalHealth(EGProjectTeam::Blue);

	FinishRoundWithResult(
		RoundResult,
		ERoundEndReason::Elimination,
		RedTeamTotalHP,
		BlueTeamTotalHP
	);
}

void AGProjectGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpawnBase::StaticClass(), (TArray<AActor*>&)SpawnZones);
}

void AGProjectGameMode::SpawnRandomItem()
{
    int32 RealActiveCount = 0;

    if (ItemPool.Num() > 0)
    {
        for (UItemSpawnDataAsset* Asset : ItemPool)
        {
            if (!Asset || !Asset->ItemClass) continue;

            TArray<AActor*> FoundItems;
        	
            UGameplayStatics::GetAllActorsOfClass(GetWorld(), Asset->ItemClass, FoundItems);

            for (AActor* Act : FoundItems)
            {
                if (::IsValid(Act) && !Act->IsActorBeingDestroyed())
                {
                    RealActiveCount++;
                }
            }
        }
    }
    CurrentSpawnedItems = RealActiveCount;
	
    if (CurrentSpawnedItems >= MaxSpawnedItems)
    {
        UE_LOG(LogTemp, Warning, TEXT("아이템 최대 스폰 갯수 도달: 맵에 이미 %d개가 존재하여 스폰하지 않습니다. (최대: %d)"), CurrentSpawnedItems, MaxSpawnedItems);
        return;
    }
	
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
          UE_LOG(LogTemp, Log, TEXT("아이템 스폰 성공! 현재 카운트: %d / %d"), CurrentSpawnedItems, MaxSpawnedItems);
       }
    }
}

void AGProjectGameMode::DecreaseSpawnedItemCount()
{
	CurrentSpawnedItems = FMath::Max(0, CurrentSpawnedItems - 1);
}

void AGProjectGameMode::HandleSeamlessTravelPlayer(AController*& Controller)
{
	Super::HandleSeamlessTravelPlayer(Controller);

	APlayerController* PC = Cast<APlayerController>(Controller);

	AssignTeam(PC);

	AGProjectPlayerState* PS = Controller->GetPlayerState<AGProjectPlayerState>();
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

void AGProjectGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	AssignTeam(NewPlayer);

	Super::HandleStartingNewPlayer_Implementation(NewPlayer);
}

bool AGProjectGameMode::ShouldSpawnAtStartSpot(AController* Player)
{
	const AGProjectPlayerState* PlayerState = Player ? Player->GetPlayerState<AGProjectPlayerState>() : nullptr;
	if (PlayerState && PlayerState->GetTeam() != EGProjectTeam::None)
	{
		// Login chooses a provisional start before PostLogin assigns the team.
		// Force team-assigned players back through ChoosePlayerStart instead.
		return false;
	}

	return Super::ShouldSpawnAtStartSpot(Player);
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

	GetWorldTimerManager().ClearTimer(
		RoundResultTimerHandle
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
	GS->SetRemainMatchTime(RoundDuration);
	
	GS->SetRoundDuration(RoundDuration);

	GetWorldTimerManager().ClearTimer(MatchTimerHandle);
	GetWorldTimerManager().ClearTimer(ItemSpawnTimerHandle);
	
	ClearPreviousRoundItems();
	
	if (ItemPool.Num() > 0 && SpawnZones.Num() > 0)
	{
		for (int32 i = 0; i < StartItemSpawnCount; ++i)
		{
			SpawnRandomItem();
		}
		
		GetWorldTimerManager().SetTimer(
			ItemSpawnTimerHandle,
			this,
			&AGProjectGameMode::SpawnRandomItem,
			ItemSpawnInterval,
			true
		);
	}

	GetWorldTimerManager().ClearTimer(MatchTimerHandle);

	GetWorldTimerManager().SetTimer(
	   MatchTimerHandle,
	   this,
	   &ThisClass::TickMatchTimer,
	   1.0f,
	   true
	);
}

void AGProjectGameMode::FinishRoundByHealth(const ERoundEndReason Reason)
{
	const float RedTeamTotalHP = CalculateTeamTotalHealth(EGProjectTeam::Red);

	const float BlueTeamTotalHP = CalculateTeamTotalHealth(EGProjectTeam::Blue);

	const ERoundResult Result =
		DetermineResultFromHealth(
			RedTeamTotalHP,
			BlueTeamTotalHP
		);

	FinishRoundWithResult(
		Result,
		Reason,
		RedTeamTotalHP,
		BlueTeamTotalHP
	);
}

void AGProjectGameMode::FinishRoundWithResult(
	const ERoundResult Result,
	const ERoundEndReason Reason,
	const float RedTeamTotalHP,
	const float BlueTeamTotalHP)
{
	if (!HasAuthority())
	{
		return;
	}

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

	GetWorldTimerManager().ClearTimer(RoundTransitionTimerHandle);

	GetWorldTimerManager().ClearTimer(RoundResultTimerHandle);

	if (Result == ERoundResult::RedWin)
	{
		GS->AddTeamRoundWin(EGProjectTeam::Red);
	}
	else if (Result == ERoundResult::BlueWin)
	{
		GS->AddTeamRoundWin(EGProjectTeam::Blue);
	}

	FGProjectRoundResultData ResultData;
	ResultData.Result = Result;
	ResultData.Reason = Reason;
	ResultData.RedTeamTotalHP = RedTeamTotalHP;
	ResultData.BlueTeamTotalHP = BlueTeamTotalHP;
	ResultData.RedTeamRoundWins = GS->GetRedTeamRoundWins();
	ResultData.BlueTeamRoundWins = GS->GetBlueTeamRoundWins();
	ResultData.Round = GS->GetCurrentRound();

	GS->SetRoundPhase(ERoundPhase::RoundResult);

	GS->BroadcastRoundResult(ResultData);

	GetWorldTimerManager().SetTimer(
		RoundResultTimerHandle,
		this,
		&ThisClass::ContinueAfterRoundResult,
		RoundResultDisplayDuration,
		false
	);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[RoundResult] Round=%d Result=%d Reason=%d RedHP=%.1f BlueHP=%.1f Score=%d:%d"),
		ResultData.Round,
		static_cast<int32>(ResultData.Result),
		static_cast<int32>(ResultData.Reason),
		ResultData.RedTeamTotalHP,
		ResultData.BlueTeamTotalHP,
		ResultData.RedTeamRoundWins,
		ResultData.BlueTeamRoundWins
	);
}

void AGProjectGameMode::ContinueAfterRoundResult()
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

float AGProjectGameMode::CalculateTeamTotalHealth(const EGProjectTeam Team) const
{
	const AGProjectGameState* GS = GetGameState<AGProjectGameState>();

	if (!GS || Team == EGProjectTeam::None)
	{
		return 0.0f;
	}

	float TotalHealth = 0.0f;

	for (APlayerState* BasePlayerState : GS->PlayerArray)
	{
		const AGProjectPlayerState* PS = Cast<AGProjectPlayerState>(BasePlayerState);

		if (!PS || PS->GetTeam() != Team)
		{
			continue;
		}

		const AGProjectCharacter* Character = Cast<AGProjectCharacter>(PS->GetPawn());

		if (Character && Character->IsDead())
		{
			continue;
		}

		const UGProjectAttributeSet* AttributeSet = PS->GetAttributeSet();

		if (!AttributeSet)
		{
			continue;
		}

		TotalHealth += FMath::Max(AttributeSet->GetHealth(), 0.0f);
	}

	return TotalHealth;
}

ERoundResult AGProjectGameMode::DetermineResultFromHealth(const float RedTeamTotalHP, const float BlueTeamTotalHP) const
{
	if (FMath::IsNearlyEqual(
		RedTeamTotalHP,
		BlueTeamTotalHP,
		0.01f
	))
	{
		return ERoundResult::Draw;
	}

	return RedTeamTotalHP > BlueTeamTotalHP
		? ERoundResult::RedWin
		: ERoundResult::BlueWin;
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
	GS->SetRoundDuration(RoundDuration);
	
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
	FinishRoundByHealth(ERoundEndReason::TimeUp);
}

void AGProjectGameMode::ClearPreviousRoundItems()
{
	CurrentSpawnedItems = 0;
	
	if (ItemPool.Num() == 0) return;

	int32 DestroyedCount = 0;
	
	for (UItemSpawnDataAsset* Asset : ItemPool)
	{
		if (!Asset || !Asset->ItemClass) continue;

		TArray<AActor*> FoundItems;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), Asset->ItemClass, FoundItems);

		for (AActor* Item : FoundItems)
		{
			if (::IsValid(Item))
			{
				Item->Destroy(); 
				DestroyedCount++;
			}
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("=== [새 라운드 청소] 실제 데이터 에셋 기반 아이템 %d개 완벽 파괴 완료! ==="), DestroyedCount);
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

	FinishRoundByHealth(ERoundEndReason::TimeUp);
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

	for (TActorIterator<AGProjectCageActor> It(GetWorld()); It; ++It)
	{
		if (AGProjectCageActor* CageActor = *It)
		{
			CageActor->ResetCageForNewRound();
		}
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

void AGProjectGameMode::OpenCagesForTeam(EGProjectTeam Team)
{
	if (!HasAuthority())
	{
		return;
	}

	if (Team == EGProjectTeam::None)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<AGProjectCageActor> It(World); It; ++It)
	{
		AGProjectCageActor* CageActor = *It;
		if (!IsValid(CageActor))
		{
			continue;
		}

		if (!CageActor->ShouldOpenWhenTeamMemberDies())
		{
			continue;
		}

		if (CageActor->GetCageTeam() != Team)
		{
			continue;
		}

		CageActor->OpenDoor();
	}
}
