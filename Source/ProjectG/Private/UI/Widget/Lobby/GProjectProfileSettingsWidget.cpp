// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/Lobby/GProjectProfileSettingsWidget.h"

#include "Components/EditableText.h"
#include "Components/Button.h"
#include "Player/Lobby/GProjectLobbyPlayerController.h"
#include "Subsystem/GProjectSessionSubsystem.h"

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
		const FText InputName = PlayerNameInput->GetText();

		if (!InputName.IsEmpty())
		{
			// +Save name code
		}
	}
	SetVisibility(ESlateVisibility::Collapsed);
}