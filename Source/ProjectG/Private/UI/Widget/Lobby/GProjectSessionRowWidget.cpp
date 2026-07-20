// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/Lobby/GProjectSessionRowWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void UGProjectSessionRowWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	// Keep the legacy row untouched. The duplicated widget is styled to match
	// WBP_Menu_New's dark arcade/cyan visual language in both designer preview
	// and at runtime.
	if (!GetClass()->GetName().Contains(TEXT("BP_SessionRow_New")))
	{
		return;
	}

	const FLinearColor PrimaryText(0.82f, 0.92f, 1.0f, 1.0f);
	const FLinearColor Accent(0.08f, 0.72f, 0.92f, 1.0f);
	const FLinearColor Panel(0.025f, 0.09f, 0.15f, 0.96f);
	const FLinearColor Hovered(0.04f, 0.24f, 0.34f, 1.0f);

	auto StyleLabel = [&PrimaryText](UTextBlock* Label)
	{
		if (!Label)
		{
			return;
		}

		Label->SetColorAndOpacity(FSlateColor(PrimaryText));
		FSlateFontInfo Font = Label->GetFont();
		Font.Size = 20;
		Font.OutlineSettings.OutlineSize = 1;
		Font.OutlineSettings.OutlineColor = FLinearColor(0.0f, 0.02f, 0.05f, 0.9f);
		Label->SetFont(Font);
	};

	StyleLabel(RoomNameText);
	StyleLabel(MapNameText);

	if (JoinButton)
	{
		FButtonStyle Style = JoinButton->GetStyle();
		Style.Normal.TintColor = FSlateColor(Panel);
		Style.Hovered.TintColor = FSlateColor(Hovered);
		Style.Pressed.TintColor = FSlateColor(Accent);
		Style.NormalPadding = FMargin(14.0f, 7.0f);
		Style.PressedPadding = FMargin(14.0f, 8.0f, 14.0f, 6.0f);
		JoinButton->SetStyle(Style);
		JoinButton->SetColorAndOpacity(Accent);
	}
}

void UGProjectSessionRowWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &UGProjectSessionRowWidget::HandleJoinButtonClicked);
	}
}

void UGProjectSessionRowWidget::SetupSessionRow(int32 InSessionIndex, const FString& InRoomName, const FString& InMapName)
{
	SessionIndex = InSessionIndex;

	if (RoomNameText)
	{
		RoomNameText->SetText(FText::FromString(InRoomName));
	}

	if (MapNameText)
	{
		MapNameText->SetText(FText::FromString(InMapName));
	}
}

void UGProjectSessionRowWidget::HandleJoinButtonClicked()
{
	if (OnSessionRowClicked.IsBound() && SessionIndex != INDEX_NONE)
	{
		OnSessionRowClicked.Broadcast(SessionIndex);
	}
}
