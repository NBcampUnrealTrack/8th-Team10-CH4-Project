// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/GProjectKillFeedEntryWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "Player/GProjectPlayerColors.h"
#include "Player/GProjectPlayerState.h"
#include "TimerManager.h"

namespace
{
const FName KillerNameBackplateName = TEXT("KillerNameBackplate");
const FName VictimNameBackplateName = TEXT("VictimNameBackplate");

FLinearColor GetTeamBackplateColor(const EGProjectTeam Team)
{
	switch (Team)
	{
	case EGProjectTeam::Red:
		return FLinearColor(0.79f, 0.06f, 0.15f, 0.90f);

	case EGProjectTeam::Blue:
		return FLinearColor(0.06f, 0.36f, 1.00f, 0.90f);

	case EGProjectTeam::None:
	default:
		return FLinearColor(0.08f, 0.08f, 0.11f, 0.85f);
	}
}

EGProjectTeam FindTeamByPlayerName(const UWorld* World, const FString& PlayerName)
{
	const AGameStateBase* GameState = World ? World->GetGameState() : nullptr;
	if (!GameState)
	{
		return EGProjectTeam::None;
	}

	for (APlayerState* BasePlayerState : GameState->PlayerArray)
	{
		const AGProjectPlayerState* PlayerState =
			Cast<AGProjectPlayerState>(BasePlayerState);

		if (PlayerState && PlayerState->GetPlayerName() == PlayerName)
		{
			return PlayerState->GetTeam();
		}
	}

	return EGProjectTeam::None;
}

void ApplyBackplateColor(
	UWidgetTree* WidgetTree,
	const FName BackplateName,
	const EGProjectTeam Team)
{
	if (!WidgetTree)
	{
		return;
	}

	if (UBorder* Backplate = Cast<UBorder>(
		WidgetTree->FindWidget(BackplateName)))
	{
		Backplate->SetBrushColor(
			GetTeamBackplateColor(Team)
		);
	}
}
}

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

	ApplyBackplateColor(
		WidgetTree,
		KillerNameBackplateName,
		FindTeamByPlayerName(World, KillerName)
	);

	ApplyBackplateColor(
		WidgetTree,
		VictimNameBackplateName,
		FindTeamByPlayerName(World, VictimName)
	);

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
