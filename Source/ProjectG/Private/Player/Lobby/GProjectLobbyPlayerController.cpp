// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/Lobby/GProjectLobbyPlayerController.h"

#include "Player/GProjectPlayerState.h"
#include "UI/Widget/GProjectLobbyWidget.h"
#include "Subsystem/GProjectSessionSubsystem.h"
#include "Subsystem/GProjectPlayerInfoSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"

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

	LobbyWidgetInstance = CreateWidget<UGProjectLobbyWidget>(this, LobbyWidgetClass);
	CurrentWidgetInstance = LobbyWidgetInstance;

	if (!CurrentWidgetInstance) return;

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