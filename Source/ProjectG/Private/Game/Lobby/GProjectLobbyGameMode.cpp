// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/Lobby/GProjectLobbyGameMode.h"
#include "Player/GProjectPlayerState.h"
#include "Player/Lobby/GProjectLobbyPlayerController.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystem/GProjectSessionSubsystem.h"
#include "OnlineSubsystem.h"
#include "Actor/PlayerStartSlot/GProjectLobbyPlayerStartSlot.h"

AGProjectLobbyGameMode::AGProjectLobbyGameMode()
{
	bUseSeamlessTravel = true;
}

void AGProjectLobbyGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	int32 CustomMaxPlayers = UGameplayStatics::GetIntOption(Options, TEXT("MaxPlayersCustom"), 0);
	if (CustomMaxPlayers >= 2)
	{
		RequiredPlayers = CustomMaxPlayers;
	}

	FString CustomMapPath = UGameplayStatics::ParseOption(Options, TEXT("BATTLE_MAP_PATH"));
	if (!CustomMapPath.IsEmpty())
	{
		BattleMapPath = CustomMapPath;
	}
}

void AGProjectLobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (AGProjectPlayerState* PS = NewPlayer->GetPlayerState<AGProjectPlayerState>())
	{
		if (NewPlayer->GetNetConnection() == nullptr)
		{
			PS->SetPlayerLobbyStatus(EGProjectPlayerLobbyStatus::Master);
		}
	}

	RefreshLobbyStateAndUI();

	//CheckAutoStart();
}

void AGProjectLobbyGameMode::HandleSeamlessTravelPlayer(AController*& Controller)
{
	Super::HandleSeamlessTravelPlayer(Controller);

	if (AGProjectPlayerState* PS = Controller->GetPlayerState<AGProjectPlayerState>())
	{
		if (Controller->GetNetConnection() == nullptr)
		{
			PS->SetPlayerLobbyStatus(EGProjectPlayerLobbyStatus::Master);
		}
	}

	RefreshLobbyStateAndUI();

	//CheckAutoStart();
}

void AGProjectLobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	RefreshAllSlots();
	UpdatePlayerCountUI();

	RefreshLobbyStateAndUI();
}

void AGProjectLobbyGameMode::BeginPlay()
{
	Super::BeginPlay();

	InitializeLobbySlots();
	UpdatePlayerCountUI();

	RefreshLobbyStateAndUI();
}

void AGProjectLobbyGameMode::UpdatePlayerCountUI()
{
	if (!IsCurrentMapLobby())
	{
		return;
	}

	if (!IsCurrentMapLobby())
	{
		return;
	}

	if (!GameState)
	{
		return;
	}

	int32 CurrentPlayers = GameState->PlayerArray.Num();
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC)
		{
			AGProjectLobbyPlayerController* LobbyPC = Cast<AGProjectLobbyPlayerController>(PC);
			if (LobbyPC)
			{
				LobbyPC->ClientUpdatePlayerCount(CurrentPlayers, RequiredPlayers);
			}
		}
	}

	for (AGProjectLobbyPlayerStartSlot* Slot : LobbySlots)
	{
		if (Slot)
		{
			Slot->RefreshSlotUI();
		}
	}
}

void AGProjectLobbyGameMode::CheckAutoStart()
{
	/*if (!GameState) return;
	if (bIsStartingGame) return;

	UWorld* World = GetWorld();
	if (!World) return;

	int32 CurrentPlayers = 0;
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		if (It->Get())
		{
			CurrentPlayers++;
		}
	}
	if (CurrentPlayers >= RequiredPlayers)
	{
		StartGame();
	}*/
}

void AGProjectLobbyGameMode::StartGame()
{
	if (!IsCurrentMapLobby())
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("StartGame Called"));

	if (bIsStartingGame)
	{
		UE_LOG(LogTemp, Warning, TEXT("Blocked: bIsStartingGame true"));
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("Blocked: World null"));
		return;
	}

	FString FinalBattleMapPath = BattleMapPath;

	UE_LOG(LogTemp, Warning, TEXT("BattleMapPath: %s"), *BattleMapPath);
	UE_LOG(LogTemp, Warning, TEXT("FinalBattleMapPath: %s"), *FinalBattleMapPath);

	if (FinalBattleMapPath.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("Blocked: FinalBattleMapPath empty"));
		return;
	}

	bIsStartingGame = true;

	FString TravelURL = FString::Printf(TEXT("%s?listen"), *FinalBattleMapPath);
	UE_LOG(LogTemp, Warning, TEXT("ServerTravel URL: %s"), *TravelURL);

	World->ServerTravel(TravelURL);
}

int32 AGProjectLobbyGameMode::GetRequiredPlayers() const
{
	return RequiredPlayers;
}

bool AGProjectLobbyGameMode::CanStartGame() const
{
	if (!IsCurrentMapLobby())
	{
		return false;
	}

	if (!GameState)
	{
		return false;
	}

	int32 TotalPlayers = GameState->PlayerArray.Num();

	if (TotalPlayers <= 1)
	{
		return false;
	}

	for (APlayerState* PS : GameState->PlayerArray)
	{
		if (AGProjectPlayerState* GProjectPS = Cast<AGProjectPlayerState>(PS))
		{
			if (GProjectPS->GetPlayerLobbyStatus() == EGProjectPlayerLobbyStatus::Master)
			{
				continue;
			}

			if (GProjectPS->GetPlayerLobbyStatus() != EGProjectPlayerLobbyStatus::Ready)
			{
				return false;
			}
		}
	}

	return true;
}

void AGProjectLobbyGameMode::InitializeLobbySlots()
{
	if (!IsCurrentMapLobby())
	{
		return;
	}

	LobbySlots.Empty();

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGProjectLobbyPlayerStartSlot::StaticClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		if (AGProjectLobbyPlayerStartSlot* SlotActor = Cast<AGProjectLobbyPlayerStartSlot>(Actor))
		{
			LobbySlots.Add(SlotActor);
		}
	}

	LobbySlots.Sort([](const AGProjectLobbyPlayerStartSlot& A, const AGProjectLobbyPlayerStartSlot& B)
		{
		return A.SlotIndex < B.SlotIndex;
		});
}

void AGProjectLobbyGameMode::RefreshAllSlots()
{
	if (!IsCurrentMapLobby())
	{
		return;
	}

	if (!GameState)
	{
		return;
	}

	for (AGProjectLobbyPlayerStartSlot* Slot : LobbySlots)
	{
		if (Slot)
		{
			Slot->UnlinkPlayer();
		}
	}

	int32 ClientSlotCounter = 1;

	for (int32 i = 0; i < GameState->PlayerArray.Num(); ++i)
	{
		if (AGProjectPlayerState* PS = Cast<AGProjectPlayerState>(GameState->PlayerArray[i]))
		{
			int32 TargetIndex = 0;

			if (PS->GetPlayerLobbyStatus() == EGProjectPlayerLobbyStatus::Master)
			{
				TargetIndex = 0;
			}
			else
			{
				TargetIndex = ClientSlotCounter;
				ClientSlotCounter++;
			}

			PS->SetSlotIndex(TargetIndex);

			if (LobbySlots.IsValidIndex(TargetIndex) && LobbySlots[TargetIndex])
			{
				LobbySlots[TargetIndex]->LinkPlayer(PS);

				if (APlayerController* PC = Cast<APlayerController>(PS->GetOwner()))
				{
					MovePlayerToSlot(PC, TargetIndex);
				}
			}
		}
	}
}

void AGProjectLobbyGameMode::MovePlayerToSlot(APlayerController* PC, int32 TargetSlotIndex)
{
	if (!IsValid(PC)) return;

	APawn* ControlledPawn = PC->GetPawn();

	if (!IsValid(ControlledPawn))
	{
		TWeakObjectPtr<APlayerController> WeakPC = PC;

		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimerForNextTick(
				FTimerDelegate::CreateWeakLambda(this, [this, WeakPC, TargetSlotIndex]()
					{
						if (!WeakPC.IsValid()) return;

						MovePlayerToSlot(WeakPC.Get(), TargetSlotIndex);
					})
			);
		}

		return;
	}

	if (!LobbySlots.IsValidIndex(TargetSlotIndex)) return;

	AGProjectLobbyPlayerStartSlot* TargetSlot = LobbySlots[TargetSlotIndex];
	if (!IsValid(TargetSlot)) return;
	if (!IsValid(TargetSlot->SpawnPoint)) return;

	ControlledPawn->SetActorLocationAndRotation(
		TargetSlot->SpawnPoint->GetComponentLocation(),
		TargetSlot->SpawnPoint->GetComponentRotation()
	);
}

bool AGProjectLobbyGameMode::IsCurrentMapLobby() const
{
	if (UWorld* World = GetWorld())
	{
		FString CurrentMapName = World->GetMapName();
		return CurrentMapName.Contains(TEXT("LobbyMap"));
	}
	return false;
}

void AGProjectLobbyGameMode::RefreshLobbyStateAndUI()
{
	UWorld* World = GetWorld();
	if (!World) return;

	World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &AGProjectLobbyGameMode::RefreshAllSlots));
	World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &AGProjectLobbyGameMode::UpdatePlayerCountUI));
}