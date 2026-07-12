// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/Lobby/GProjectMenuWidget.h"
#include "Subsystem/GProjectSessionSubsystem.h"
#include "UI/Widget/Lobby/GProjectSessionRowWidget.h"
#include "Game/Lobby/GProjectLobbyGameMode.h"
#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Components/ScrollBox.h"
#include "Components/Border.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Player/Lobby/GProjectLobbyPlayerController.h"
#include "Kismet/GameplayStatics.h"

UGProjectMenuWidget::UGProjectMenuWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UGProjectMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	JoinButton->OnClicked.AddDynamic(this, &ThisClass::OnJoinButtonClicked);
	ExitButton->OnClicked.AddDynamic(this, &ThisClass::OnExitButtonClicked);
	HostButton->OnClicked.AddDynamic(this, &ThisClass::OnHostButtonClicked);

	if (ConfirmNoResultsButton)
	{
		ConfirmNoResultsButton->OnClicked.AddDynamic(this, &ThisClass::OnConfirmNoResultsClicked);
	}

	if (NoResultsBorder)
	{
		NoResultsBorder->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UGProjectMenuWidget::OnHostButtonClicked()
{
	int32 TargetMaxPlayers = 2;

	if (UWorld* World = GetWorld())
	{
		if (AWorldSettings* WorldSettings = World->GetWorldSettings())
		{
			if (UClass* GameModeClass = WorldSettings->DefaultGameMode)
			{
				if (const AGProjectLobbyGameMode* DefaultGM =
					Cast<AGProjectLobbyGameMode>(GameModeClass->GetDefaultObject()))
				{
					TargetMaxPlayers = DefaultGM->GetRequiredPlayers();
				}
			}
		}
	}

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		UGProjectSessionSubsystem* Subsystem =
			GameInstance->GetSubsystem<UGProjectSessionSubsystem>();

		if (Subsystem)
		{
			Subsystem->CreateGameSession(TargetMaxPlayers, FName("LobbyMap"));
		}
	}
}

void UGProjectMenuWidget::OnJoinButtonClicked()
{
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        UGProjectSessionSubsystem* Subsystem = GameInstance->GetSubsystem<UGProjectSessionSubsystem>();
        if (Subsystem)
        {
            Subsystem->OnFindSessionsCompleteEvent.RemoveDynamic(this, &ThisClass::OnFindSessionsCompleteUpdateUI);
            Subsystem->OnFindSessionsCompleteEvent.AddDynamic(this, &ThisClass::OnFindSessionsCompleteUpdateUI);
            Subsystem->FindGameSessions();
        }
    }
}

void UGProjectMenuWidget::OnExitButtonClicked()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}

void UGProjectMenuWidget::OnFindSessionsCompleteUpdateUI(const TArray<FString>& SessionNames, bool bWasSuccessful)
{
	if (!SessionListScrollBox || !SessionRowWidgetClass) return;

	SessionListScrollBox->ClearChildren();

	if (bWasSuccessful && SessionNames.Num() > 0)
	{
		if (NoResultsBorder)
		{
			NoResultsBorder->SetVisibility(ESlateVisibility::Collapsed);
		}

		if (UGameInstance* GameInstance = GetGameInstance())
		{
			UGProjectSessionSubsystem* Subsystem = GameInstance->GetSubsystem<UGProjectSessionSubsystem>();
			if (Subsystem)
			{
				for (int32 i = 0; i < SessionNames.Num(); ++i)
				{
					UGProjectSessionRowWidget* RowWidget = CreateWidget<UGProjectSessionRowWidget>(this, SessionRowWidgetClass);
					if (!RowWidget) continue;

					FString RoomName = SessionNames[i];
					int32 CurrentPlayers = 1;
					int32 MaxPlayers = 2;

					Subsystem->GetSessionPlayerCounts(i, CurrentPlayers, MaxPlayers);

					RowWidget->SetupSessionRow(i, RoomName, CurrentPlayers, MaxPlayers);

					RowWidget->OnSessionRowClicked.RemoveDynamic(this, &ThisClass::HandleSessionRowClicked);
					RowWidget->OnSessionRowClicked.AddDynamic(this, &ThisClass::HandleSessionRowClicked);

					SessionListScrollBox->AddChild(RowWidget);
				}
			}
		}
	}
	else
	{
		if (NoResultsBorder)
		{
			NoResultsBorder->SetVisibility(ESlateVisibility::Visible);
		}
	}
}

void UGProjectMenuWidget::HandleSessionRowClicked(int32 SessionIndex)
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		UGProjectSessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UGProjectSessionSubsystem>();
		if (SessionSubsystem)
		{
			SessionSubsystem->JoinGameSession(SessionIndex);
		}
	}
}

void UGProjectMenuWidget::OnConfirmNoResultsClicked()
{
	if (NoResultsBorder)
	{
		NoResultsBorder->SetVisibility(ESlateVisibility::Collapsed);
	}
}