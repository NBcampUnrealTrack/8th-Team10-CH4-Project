// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/GProjectMatchResultWidget.h"

#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Player/GProjectPlayerController.h"
#include "Subsystem/GProjectSessionSubsystem.h"

void UGProjectMatchResultWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ReturnButton)
	{
		ReturnButton->OnClicked.AddDynamic(this, &ThisClass::OnReturnButtonClicked);
	}
	if (ExitButton)
	{
		ExitButton->OnClicked.AddDynamic(this, &ThisClass::OnExitButtonClicked);
	}
}

void UGProjectMatchResultWidget::ShowResult(
	bool bVictory,
	int32 RedTeamWins,
	int32 BlueTeamWins)
{
	SetVisibility(ESlateVisibility::Visible);

	if (ResultText)
	{
		if (bVictory)
		{
			ResultText->SetText(
				NSLOCTEXT(
					"MatchResult",
					"Victory",
					"VICTORY"
				)
			);

			ResultText->SetColorAndOpacity(
				FSlateColor(
					FLinearColor::FromSRGBColor(
						FColor(255, 225, 100, 255)
					)
				)
			);
		}
		else
		{
			ResultText->SetText(
				NSLOCTEXT(
					"MatchResult",
					"Defeat",
					"DEFEAT"
				)
			);

			ResultText->SetColorAndOpacity(
				FSlateColor(
					FLinearColor::FromSRGBColor(
						FColor(255, 110, 120, 255)
					)
				)
			);
		}
	}

	if (FinalScoreText)
	{
		FinalScoreText->SetText(
			FText::Format(
				NSLOCTEXT(
					"MatchResult",
					"FinalScore",
					"RED {0} : {1} BLUE"
				),
				FText::AsNumber(RedTeamWins),
				FText::AsNumber(BlueTeamWins)
			)
		);
	}

	if (APlayerController* PC = GetOwningPlayer())
	{
		if (PC->HasAuthority())
		{
			if (ReturnButton) ReturnButton->SetVisibility(ESlateVisibility::Visible);
			if (WaitingHostText) WaitingHostText->SetVisibility(ESlateVisibility::Collapsed);
		}
		else
		{
			if (ReturnButton) ReturnButton->SetVisibility(ESlateVisibility::Collapsed);
			if (WaitingHostText)
			{
				WaitingHostText->SetVisibility(ESlateVisibility::Visible);
			}
		}

		PC->SetShowMouseCursor(true);
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(TakeWidget());
		PC->SetInputMode(InputMode);
	}
}

void UGProjectMatchResultWidget::HideResult()
{
	SetVisibility(ESlateVisibility::Collapsed);
}

void UGProjectMatchResultWidget::OnReturnButtonClicked()
{
	if (AGProjectPlayerController* PC = Cast<AGProjectPlayerController>(GetOwningPlayer()))
	{
		PC->ServerRequestReturnToLobby();
	}
}

void UGProjectMatchResultWidget::OnExitButtonClicked()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UGProjectSessionSubsystem* SessionSubsystem = GI->GetSubsystem<UGProjectSessionSubsystem>())
		{
			SessionSubsystem->ExitMatch(GetOwningPlayer());
		}
	}
}