// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/Lobby/GProjectMenuWidget.h"
#include "Subsystem/GProjectSessionSubsystem.h"
#include "UI/Widget/Lobby/GProjectSessionRowWidget.h"
#include "Game/Lobby/GProjectLobbyGameMode.h"
#include "Game/Lobby/GProjectSessionTypes.h"
#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Components/ScrollBox.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
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

	if (ConfirmHostButton)
	{
		ConfirmHostButton->OnClicked.AddDynamic(this, &ThisClass::OnConfirmHostButtonClicked);
	}

	if (HostSettingsBorder)
	{
		HostSettingsBorder->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (BtnPrevMap)
	{
		BtnPrevMap->OnClicked.AddDynamic(this, &ThisClass::OnPrevMapClicked);
	}

	if (BtnNextMap)
	{
		BtnNextMap->OnClicked.AddDynamic(this, &ThisClass::OnNextMapClicked);
	}

	if (BtnPrevPlayers)
	{
		BtnPrevPlayers->OnClicked.AddDynamic(this, &ThisClass::OnPrevPlayersClicked);
	}

	if (BtnNextPlayers)
	{
		BtnNextPlayers->OnClicked.AddDynamic(this, &ThisClass::OnNextPlayersClicked);
	}

	if (ProfileSettingsButton)
	{
		ProfileSettingsButton->OnClicked.AddDynamic(this, &ThisClass::OnProfileSettingsButtonClicked);
	}

	InitMapDataFromTable();
	UpdateMapSelectionUI();
	UpdatePlayerSelectionUI();
}

void UGProjectMenuWidget::OnHostButtonClicked()
{
	if (HostSettingsBorder)
	{
		HostSettingsBorder->SetVisibility(ESlateVisibility::Visible);
	}
}

void UGProjectMenuWidget::OnConfirmHostButtonClicked()
{
	FString RoomName = RoomNameInput ? RoomNameInput->GetText().ToString() : TEXT("New Room");
	FString TargetMapName = TEXT("DefaultMap");
	FString TargetMapPath = TEXT("/Game/Level/TestLevel");

	if (CachedBattleMaps.IsValidIndex(CurrentMapIndex))
	{
		TargetMapName = CachedBattleMaps[CurrentMapIndex].DisplayName;
		TargetMapPath = CachedBattleMaps[CurrentMapIndex].MapPath;
	}

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		UGProjectSessionSubsystem* Subsystem = GameInstance->GetSubsystem<UGProjectSessionSubsystem>();
		if (Subsystem)
		{
			Subsystem->CreateGameSession(CurrentMaxPlayers, FName(*TargetMapName), RoomName, TargetMapPath);
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

void UGProjectMenuWidget::InitMapDataFromTable()
{
	CachedBattleMaps.Empty();

	if (!MapDataTable)
	{
		return;
	}

	TArray<FBattleMapData*> AllRows;
	MapDataTable->GetAllRows<FBattleMapData>(TEXT("GProjectMenuWidgetContext"), AllRows);

	for (FBattleMapData* Row : AllRows)
	{
		if (Row)
		{
			CachedBattleMaps.Add(*Row);
		}
	}
}

void UGProjectMenuWidget::OnPrevMapClicked()
{
	if (CachedBattleMaps.Num() == 0)
	{
		return;
	}

	CurrentMapIndex--;

	if (CurrentMapIndex < 0)
	{
		CurrentMapIndex = CachedBattleMaps.Num() - 1;
	}

	UpdateMapSelectionUI();
}

void UGProjectMenuWidget::OnNextMapClicked()
{
	if (CachedBattleMaps.Num() == 0)
	{
		return;
	}

	CurrentMapIndex++;

	if (CurrentMapIndex >= CachedBattleMaps.Num())
	{
		CurrentMapIndex = 0;
	}

	UpdateMapSelectionUI();
}

void UGProjectMenuWidget::OnPrevPlayersClicked()
{
	CurrentMaxPlayers--;

	if (CurrentMaxPlayers < 2)
	{
		CurrentMaxPlayers = 4;
	}

	UpdatePlayerSelectionUI();
}

void UGProjectMenuWidget::OnNextPlayersClicked()
{
	CurrentMaxPlayers++;

	if (CurrentMaxPlayers > 4)
	{
		CurrentMaxPlayers = 2;
	}

	UpdatePlayerSelectionUI();
}

void UGProjectMenuWidget::UpdateMapSelectionUI()
{
	if (!TextMapName)
	{
		return;
	}

	if (CachedBattleMaps.IsValidIndex(CurrentMapIndex))
	{
		TextMapName->SetText(FText::FromString(CachedBattleMaps[CurrentMapIndex].DisplayName));
	}
	else
	{
		TextMapName->SetText(FText::FromString(TEXT("No Maps Available")));
	}
}

void UGProjectMenuWidget::UpdatePlayerSelectionUI()
{
	if (TextMaxPlayers)
	{
		TextMaxPlayers->SetText(FText::AsNumber(CurrentMaxPlayers));
	}
}

void UGProjectMenuWidget::OnProfileSettingsButtonClicked()
{
	if (ProfileWidgetClass)
	{
		UUserWidget* ProfileWidget = CreateWidget<UUserWidget>(GetWorld(), ProfileWidgetClass);
		if (ProfileWidget)
		{
			ProfileWidget->AddToViewport();
		}
	}
}