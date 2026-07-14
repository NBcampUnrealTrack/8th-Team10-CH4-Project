// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/GProjectKillFeedEntryWidget.h"

#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "Player/GProjectPlayerColors.h"
#include "TimerManager.h"

void UGProjectKillFeedEntryWidget::
InitializeKillFeedEntry(
	const FString& KillerName,
	const int32 KillerColorIndex,
	const FString& VictimName,
	const int32 VictimColorIndex)
{
	if (KillerNameText)
	{
		KillerNameText->SetText(
			FText::FromString(KillerName)
		);

		const FLinearColor KillerColor =
			KillerColorIndex != INDEX_NONE
			? GProjectPlayerColors::GetColor(
				KillerColorIndex
			)
			: FLinearColor::White;

		KillerNameText->SetColorAndOpacity(
			FSlateColor(KillerColor)
		);
	}

	if (VictimNameText)
	{
		VictimNameText->SetText(
			FText::FromString(VictimName)
		);

		const FLinearColor VictimColor =
			VictimColorIndex != INDEX_NONE
			? GProjectPlayerColors::GetColor(
				VictimColorIndex
			)
			: FLinearColor::White;

		VictimNameText->SetColorAndOpacity(
			FSlateColor(VictimColor)
		);
	}

	UWorld* World = GetWorld();

	if (!World || World->bIsTearingDown)
	{
		return;
	}

	World->GetTimerManager().SetTimer(
		RemoveTimerHandle,
		this,
		&ThisClass::RemoveKillFeedEntry,
		DisplayDuration,
		false
	);
}

void UGProjectKillFeedEntryWidget::
RemoveKillFeedEntry()
{
	RemoveFromParent();
}

void UGProjectKillFeedEntryWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(
			RemoveTimerHandle
		);
	}

	Super::NativeDestruct();
}