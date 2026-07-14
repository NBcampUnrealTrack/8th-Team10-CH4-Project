// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/GProjectRoundCountdownWidget.h"

#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UGProjectRoundCountdownWidget::NativeConstruct()
{
	Super::NativeConstruct();

	HideCountdown();
}

void UGProjectRoundCountdownWidget::ShowCountdown(
	const int32 CountdownValue)
{
	if (!CountdownText)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HideTimerHandle);
	}

	if (CountdownValue > 0)
	{
		CountdownText->SetText(FText::AsNumber(CountdownValue));
	}
	else if (CountdownValue == 0)
	{
		CountdownText->SetText(FText::FromString(TEXT("Fight!")));
	}
	else
	{
		HideCountdown();
		return;
	}

	SetVisibility(ESlateVisibility::HitTestInvisible);

	if (CountdownValue == 0)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(
				HideTimerHandle,
				this,
				&ThisClass::HideCountdown,
				FightDisplayDuration,
				false
			);
		}
	}
}

void UGProjectRoundCountdownWidget::HideCountdown()
{
	SetVisibility(ESlateVisibility::Collapsed);
}

void UGProjectRoundCountdownWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HideTimerHandle);
	}

	Super::NativeDestruct();
}