// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/GProjectLobbyWidget.h"

#include "Components/TextBlock.h"
#include "Components/EditableTextBox.h"
#include "Components/Button.h"
#include "Player/Lobby/GProjectLobbyPlayerController.h"
#include "Player/GProjectPlayerState.h"
#include "Game/Lobby/GProjectLobbyGameState.h"
#include "Game/Lobby/GProjectLobbyGameMode.h"

void UGProjectLobbyWidget::UpdatePlayerCountText(int32 CurrentPlayers, int32 RequiredPlayers)
{
	if (PlayerCountText)
	{
		FString ContentString = FString::Printf(TEXT("Players %d / %d"), CurrentPlayers, RequiredPlayers);
		PlayerCountText->SetText(FText::FromString(ContentString));
	}
}

void UGProjectLobbyWidget::InitLobbyWidget(AGProjectLobbyPlayerController* InPC)
{
	OwningLobbyPC = InPC;

	if (OwningLobbyPC)
	{
		OwningLobbyPC->BindLobbyWidgetToPlayerState();

		if (AGProjectPlayerState* PS = OwningLobbyPC->GetPlayerState<AGProjectPlayerState>())
		{
			PS->OnReadyChanged.RemoveAll(this);
			PS->OnReadyChanged.AddUObject(this, &UGProjectLobbyWidget::RefreshButtonState);
		}
	}
	RefreshButtonState();
}

void UGProjectLobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (LobbyActionButton)
	{
		LobbyActionButton->OnClicked.AddDynamic(this, &UGProjectLobbyWidget::OnLobbyActionClicked);
	}
	RefreshButtonState();
}

void UGProjectLobbyWidget::OnLobbyActionClicked()
{
	if (!OwningLobbyPC) return;

	if (AGProjectPlayerState* PS = OwningLobbyPC->GetPlayerState<AGProjectPlayerState>())
	{
		if (PS->bIsHost)
		{
			OwningLobbyPC->Server_RequestStartGame();
		}
		else
		{
			OwningLobbyPC->Server_ToggleReady();
		}
	}
}

void UGProjectLobbyWidget::RefreshButtonState()
{
	if (!OwningLobbyPC) return;

	AGProjectPlayerState* PS = OwningLobbyPC->GetPlayerState<AGProjectPlayerState>();
	if (!PS)
	{
		return;
	}

	if (PS->bIsHost)
	{
		if (LobbyActionText) LobbyActionText->SetText(FText::FromString(TEXT("START")));

		if (UWorld* World = GetWorld())
		{
			if (AGProjectLobbyGameMode* GM = World->GetAuthGameMode<AGProjectLobbyGameMode>())
			{
				if (LobbyActionButton) LobbyActionButton->SetIsEnabled(GM->CanStartGame());
			}
		}
	}
	else
	{
		if (LobbyActionButton) LobbyActionButton->SetIsEnabled(true);

		if (LobbyActionText)
		{
			if (PS->bIsReady)
			{
				LobbyActionText->SetText(FText::FromString(TEXT("WAIT")));
			}
			else
			{
				LobbyActionText->SetText(FText::FromString(TEXT("READY")));
			}
		}
	}
}