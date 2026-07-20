// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/GProjectLobbyWidget.h"

#include "Components/TextBlock.h"
#include "Components/EditableTextBox.h"
#include "Components/Button.h"
#include "Player/Lobby/GProjectLobbyPlayerController.h"
#include "Player/GProjectPlayerState.h"
#include "Game/Lobby/GProjectLobbyGameState.h"
#include "Game/Lobby/GProjectLobbyGameMode.h"

void UGProjectLobbyWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	// Preserve WBP_LobbyWidget's legacy presentation and apply the new menu
	// visual language only to its duplicated variant.
	if (!GetClass()->GetName().Contains(TEXT("WBP_LobbyWidget_New")))
	{
		return;
	}

	const FLinearColor PrimaryText(0.82f, 0.92f, 1.0f, 1.0f);
	const FLinearColor Accent(0.08f, 0.72f, 0.92f, 1.0f);
	const FLinearColor Panel(0.025f, 0.09f, 0.15f, 0.96f);
	const FLinearColor Hovered(0.04f, 0.24f, 0.34f, 1.0f);

	auto StyleLabel = [&PrimaryText](UTextBlock* Label, int32 FontSize)
	{
		if (!Label)
		{
			return;
		}

		Label->SetColorAndOpacity(FSlateColor(PrimaryText));
		FSlateFontInfo Font = Label->GetFont();
		Font.Size = FontSize;
		Font.OutlineSettings.OutlineSize = 1;
		Font.OutlineSettings.OutlineColor = FLinearColor(0.0f, 0.02f, 0.05f, 0.9f);
		Label->SetFont(Font);
	};

	StyleLabel(PlayerCountText, 22);
	StyleLabel(LobbyActionText, 24);

	if (LobbyActionButton)
	{
		FButtonStyle Style = LobbyActionButton->GetStyle();
		Style.Normal.TintColor = FSlateColor(Panel);
		Style.Hovered.TintColor = FSlateColor(Hovered);
		Style.Pressed.TintColor = FSlateColor(Accent);
		Style.Disabled.TintColor = FSlateColor(FLinearColor(0.025f, 0.06f, 0.08f, 0.65f));
		Style.NormalPadding = FMargin(20.0f, 10.0f);
		Style.PressedPadding = FMargin(20.0f, 11.0f, 20.0f, 9.0f);
		LobbyActionButton->SetStyle(Style);
		LobbyActionButton->SetColorAndOpacity(Accent);
	}
}

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
			PS->OnLobbyStatusChanged.RemoveAll(this);
			PS->OnLobbyStatusChanged.AddUObject(this, &UGProjectLobbyWidget::RefreshButtonState);
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
		if (PS->GetPlayerLobbyStatus() == EGProjectPlayerLobbyStatus::Master)
		{
			OwningLobbyPC->Server_RequestStartGame();
		}
		else
		{
			OwningLobbyPC->Server_ToggleReady();
		}
	}
}

void UGProjectLobbyWidget::RefreshButtonState(EGProjectPlayerLobbyStatus NewStatus)
{
	if (!OwningLobbyPC) return;

	AGProjectPlayerState* PS = OwningLobbyPC->GetPlayerState<AGProjectPlayerState>();
	if (!PS)
	{
		return;
	}

	if (PS->GetPlayerLobbyStatus() == EGProjectPlayerLobbyStatus::Master)
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
			if (PS->GetPlayerLobbyStatus() == EGProjectPlayerLobbyStatus::Ready)
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
