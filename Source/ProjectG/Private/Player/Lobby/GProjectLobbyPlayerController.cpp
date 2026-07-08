// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/Lobby/GProjectLobbyPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"

void AGProjectLobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController())
	{
		return;
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

	CurrentWidgetInstance = CreateWidget<UUserWidget>(this, LobbyWidgetClass);
	LobbyWidgetInstance = CurrentWidgetInstance;

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
	if (!LobbyWidgetInstance) return;

	UTextBlock* PlayerCountText = Cast<UTextBlock>(LobbyWidgetInstance->GetWidgetFromName(TEXT("PlayerCountText")));
	if (!PlayerCountText) return;

	FString ContentString = FString::Printf(TEXT("Players %d / %d"), CurrentPlayers, RequiredPlayers);

	PlayerCountText->SetText(FText::FromString(ContentString));
}