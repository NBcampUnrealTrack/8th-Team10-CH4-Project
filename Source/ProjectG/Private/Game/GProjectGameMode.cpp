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

	GetWorldTimerManager().ClearTimer(MatchTimerHandle);
	GetWorldTimerManager().ClearTimer(ItemSpawnTimerHandle);
	
	ClearPreviousRoundItems();
	
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
			2.0f,
			true,
			2.0f
		);
	}
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
	if (!GS) return;
	if (GS->GetRoundPhase() != ERoundPhase::Playing) return;
	
	GetWorldTimerManager().ClearTimer(MatchTimerHandle);
	GetWorldTimerManager().ClearTimer(ItemSpawnTimerHandle);

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
