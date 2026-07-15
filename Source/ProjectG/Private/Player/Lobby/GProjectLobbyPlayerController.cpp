// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/Lobby/GProjectLobbyPlayerController.h"

#include "Player/GProjectPlayerState.h"
#include "Game/Lobby/GProjectLobbyGameMode.h"
#include "UI/Widget/GProjectLobbyWidget.h"
#include "Subsystem/GProjectSessionSubsystem.h"
#include "Subsystem/GProjectPlayerInfoSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

AGProjectLobbyPlayerController::AGProjectLobbyPlayerController()
{
	bAutoManageActiveCameraTarget = false;
}

void AGProjectLobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController()) return;

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		UGProjectSessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UGProjectSessionSubsystem>();
		if (SessionSubsystem)
		{
			SessionSubsystem->LoginWithEOS();
		}

		UGProjectPlayerInfoSubsystem* PlayerInfoSubsystem = GameInstance->GetSubsystem<UGProjectPlayerInfoSubsystem>();
		if (PlayerInfoSubsystem)
		{
			ServerSetPlayerName(PlayerInfoSubsystem->GetPlayerName());
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Yellow, FString::Printf(TEXT("%s"), *PlayerInfoSubsystem->GetPlayerName()));
		}

	}

	FString CurrentMapName = GetWorld()->GetMapName();

	if (CurrentMapName.Contains(TEXT("MenuMap")))
	{
		ShowMenuUI();
		return;
	}

	if (CurrentMapName.Contains(TEXT("LobbyMap")))
	{
		ShowLobbyUI();
		return;
	}

	// Lobby Camera
	if (IsLocalController())
	{
		TArray<AActor*> FoundCameras;
		UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("LobbyCamera"), FoundCameras);

		if (FoundCameras.Num() > 0)
		{
			SetViewTargetWithBlend(FoundCameras[0], 0.0f);
		}
	}

	ClearCurrentWidget();
}

void AGProjectLobbyPlayerController::ShowMenuUI()
{
	ClearCurrentWidget();

	if (!MenuWidgetClass) return;

	CurrentWidgetInstance = CreateWidget<UUserWidget>(this, MenuWidgetClass);
	if (!CurrentWidgetInstance) return;

	CurrentWidgetInstance->AddToViewport();

	FInputModeUIOnly Mode;
	Mode.SetWidgetToFocus(CurrentWidgetInstance->GetCachedWidget());
	SetInputMode(Mode);

	bShowMouseCursor = true;
}

void AGProjectLobbyPlayerController::ShowLobbyUI()
{
	ClearCurrentWidget();

	if (!LobbyWidgetClass) return;

	UGProjectLobbyWidget* LobbyUI = CreateWidget<UGProjectLobbyWidget>(this, LobbyWidgetClass);
	if (!LobbyUI) return;

	LobbyUI->InitLobbyWidget(this);

	LobbyWidgetInstance = LobbyUI;
	CurrentWidgetInstance = LobbyUI;

	CurrentWidgetInstance->AddToViewport();

	FInputModeUIOnly Mode;
	Mode.SetWidgetToFocus(CurrentWidgetInstance->GetCachedWidget());
	SetInputMode(Mode);

	bShowMouseCursor = true;

	if (bHasPendingUpdate)
	{
		ApplyPlayerCountToLobbyWidget(PendingCurrentPlayers, PendingRequiredPlayers);
		bHasPendingUpdate = false;
	}

	TArray<AActor*> FoundCameras;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName(TEXT("LobbyCamera")), FoundCameras);

	if (FoundCameras.Num() > 0 && FoundCameras[0])
	{
		SetViewTargetWithBlend(FoundCameras[0], 0.f);
	}

	if (bHasPendingUpdate)
	{
		ApplyPlayerCountToLobbyWidget(PendingCurrentPlayers, PendingRequiredPlayers);
		bHasPendingUpdate = false;
	}
}

void AGProjectLobbyPlayerController::ClearCurrentWidget()
{
	if (CurrentWidgetInstance)
	{
		CurrentWidgetInstance->RemoveFromParent();
		CurrentWidgetInstance = nullptr;
	}

	LobbyWidgetInstance = nullptr;
}

void AGProjectLobbyPlayerController::ClientUpdatePlayerCount_Implementation(int32 CurrentPlayers, int32 RequiredPlayers)
{
	if (!LobbyWidgetInstance)
	{
		PendingCurrentPlayers = CurrentPlayers;
		PendingRequiredPlayers = RequiredPlayers;
		bHasPendingUpdate = true;
		return;
	}

	ApplyPlayerCountToLobbyWidget(CurrentPlayers, RequiredPlayers);
}

void AGProjectLobbyPlayerController::ApplyPlayerCountToLobbyWidget(int32 CurrentPlayers, int32 RequiredPlayers)
{
	if (!LobbyWidgetInstance)
	{
		return;
	}

	if (UGProjectLobbyWidget* LobbyUI = Cast<UGProjectLobbyWidget>(LobbyWidgetInstance))
	{
		LobbyUI->UpdatePlayerCountText(CurrentPlayers, RequiredPlayers);
	}
}

void AGProjectLobbyPlayerController::ServerSetPlayerName_Implementation(const FString& InName)
{
	if (AGProjectPlayerState* PS = Cast<AGProjectPlayerState>(PlayerState))
	{
		PS->SetPlayerName(InName);
	}
}

bool AGProjectLobbyPlayerController::ServerSetPlayerName_Validate(const FString& InName)
{
	return true;
}


void AGProjectLobbyPlayerController::Server_ToggleReady_Implementation()
{
	AGProjectPlayerState* PS = GetPlayerState<AGProjectPlayerState>();
	if (!PS || PS->GetPlayerLobbyStatus() == EGProjectPlayerLobbyStatus::Master) return;

	EGProjectPlayerLobbyStatus TargetStatus = (PS->GetPlayerLobbyStatus() == EGProjectPlayerLobbyStatus::Ready) ?
		EGProjectPlayerLobbyStatus::Wait : EGProjectPlayerLobbyStatus::Ready;

	PS->SetPlayerLobbyStatus(TargetStatus);

	if (GEngine)
	{
		FString StateText = (TargetStatus == EGProjectPlayerLobbyStatus::Ready) ? TEXT("READY") : TEXT("WAIT");
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("[%s] State: %s"), *PS->GetPlayerName(), *StateText));
	}

	UWorld* World = GetWorld();
	if (World)
	{
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			if (AGProjectLobbyPlayerController* LobbyPC = Cast<AGProjectLobbyPlayerController>(It->Get()))
			{
				LobbyPC->ClientRefreshLobbyUI();
			}
		}
	}
}

void AGProjectLobbyPlayerController::Server_RequestStartGame_Implementation()
{
	AGProjectPlayerState* PS = GetPlayerState<AGProjectPlayerState>();
	if (PS && PS->GetPlayerLobbyStatus() == EGProjectPlayerLobbyStatus::Master)
	{
		if (AGProjectLobbyGameMode* GM = GetWorld()->GetAuthGameMode<AGProjectLobbyGameMode>())
		{
			if (GM->CanStartGame())
			{
				if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, TEXT("Host requested Game Start!"));
				GM->StartGame();
			}
		}
	}
}

UGProjectLobbyWidget* AGProjectLobbyPlayerController::GetLobbyWidget() const
{
	return Cast<UGProjectLobbyWidget>(LobbyWidgetInstance);
}

void AGProjectLobbyPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	BindLobbyWidgetToPlayerState();
}

void AGProjectLobbyPlayerController::BindLobbyWidgetToPlayerState()
{
	if (!IsLocalController()) return;

	UGProjectLobbyWidget* LobbyUI = GetLobbyWidget();
	if (!LobbyUI) return;

	AGProjectPlayerState* PS = GetPlayerState<AGProjectPlayerState>();
	if (!PS) return;

	PS->OnLobbyStatusChanged.RemoveAll(LobbyUI);
	PS->OnLobbyStatusChanged.AddUObject(LobbyUI, &UGProjectLobbyWidget::RefreshButtonState);

	LobbyUI->RefreshButtonState();
}

void AGProjectLobbyPlayerController::ClientRefreshLobbyUI_Implementation()
{
	if (UGProjectLobbyWidget* LobbyUI = GetLobbyWidget())
	{
		LobbyUI->RefreshButtonState();
	}
}


