// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/Lobby/GProjectProfileSettingsWidget.h"

#include "Components/EditableText.h"
#include "Components/Button.h"
#include "Player/Lobby/GProjectLobbyPlayerController.h"
#include "Player/GProjectPlayerState.h"
#include "Subsystem/GProjectPlayerInfoSubsystem.h"

void UGProjectProfileSettingsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (BtnSaveProfile)
	{
		BtnSaveProfile->OnClicked.AddDynamic(this, &ThisClass::OnSaveProfileClicked);
	}
}

void UGProjectProfileSettingsWidget::OnSaveProfileClicked()
{
	if (PlayerNameInput)
	{
		const FString InputName = PlayerNameInput->GetText().ToString();

		if (!InputName.IsEmpty())
		{
			if (UGameInstance* GI = GetGameInstance())
			{
				if (UGProjectPlayerInfoSubsystem* InfoSubsystem = GI->GetSubsystem<UGProjectPlayerInfoSubsystem>())
				{
					InfoSubsystem->SetPlayerName(InputName);
					InfoSubsystem->SetPlayerName(InputName);
				}
			}
		}
	}

	SetVisibility(ESlateVisibility::Collapsed);
}