// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/GProjectChatWidget.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Components/EditableTextBox.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Player/GProjectPlayerController.h"
#include "UI/Widget/GProjectChatMessageWidget.h"	
#include "Engine/World.h"
#include "TimerManager.h"

namespace
{
	constexpr int32 MaxVisibleChatMessage = 50;
}

void UGProjectChatWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ChatInputTextBox)
	{
		ChatInputTextBox->OnTextCommitted.RemoveDynamic(
			this,
			&ThisClass::HandleTextCommitted
		);

		ChatInputTextBox->OnTextCommitted.AddDynamic(
			this,
			&ThisClass::HandleTextCommitted
		);

		ChatInputTextBox->SetClearKeyboardFocusOnCommit(false);
		ChatInputTextBox->SetVisibility(ESlateVisibility::Collapsed);
	}

	AGProjectPlayerController* PC = Cast<AGProjectPlayerController>(GetOwningPlayer());
	if (!PC)
	{
		return;
	}

	PC->RegisterChatWidget(this);


	UE_LOG(LogTemp, Warning, TEXT("ChatWidget NativeConstruct"));
}

void UGProjectChatWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HideChatTimerHandle);
	}

	if (ChatInputTextBox)
	{
		ChatInputTextBox->OnTextCommitted.RemoveDynamic(
			this,
			&ThisClass::HandleTextCommitted
		);
	}

	AGProjectPlayerController* PC = Cast<AGProjectPlayerController>(GetOwningPlayer());
	if (!PC)
	{
		return;
	}

	PC->UnRegisterChatWidget(this);

	Super::NativeDestruct();
}

void UGProjectChatWidget::AddChatMessage(int32 SenderPlayerID, const FString& SenderName, const FString& Message)
{
	if (!MessageScrollBox || !WidgetTree)
	{
		return;
	}

	UGProjectChatMessageWidget* MessageWidget = CreateWidget<UGProjectChatMessageWidget>(GetOwningPlayer(), ChatMessageWidgetClass);

	if (!MessageWidget)
	{
		return;
	}

	MessageWidget->SetMessage(SenderName, Message, GetPlayerColor(SenderPlayerID));

	MessageScrollBox->AddChild(MessageWidget);
	MessageScrollBox->ScrollToEnd();

	if (ChatHistoryPanel)
	{
		ChatHistoryPanel->SetVisibility(ESlateVisibility::Visible);
	}

	if (!bChatInputOpen)
	{
		RestartHideTimer();
	}
}

void UGProjectChatWidget::OpenChatInput()
{
	AGProjectPlayerController* PC = Cast<AGProjectPlayerController>(GetOwningPlayer());
	if (!PC || !ChatInputTextBox)
	{
		return;
	}

	bChatInputOpen = true;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(
			HideChatTimerHandle
		);
	}

	if (ChatHistoryPanel)
	{
		ChatHistoryPanel->SetVisibility(ESlateVisibility::Visible);
	}

	ChatInputTextBox->SetVisibility(ESlateVisibility::Visible);
	ChatInputTextBox->SetIsEnabled(true);
	ChatInputTextBox->SetIsReadOnly(false);
	ChatInputTextBox->SetText(FText::GetEmpty());

	UWidgetBlueprintLibrary::SetInputMode_GameAndUIEx(
		PC,
		ChatInputTextBox,
		EMouseLockMode::DoNotLock,
		true,
		false
	);

	ChatInputTextBox->SetKeyboardFocus();
}

void UGProjectChatWidget::CloseChatInput()
{
	bChatInputOpen = false;

	if (!ChatInputTextBox)
	{
		return;	
	}

	ChatInputTextBox->SetText(FText::GetEmpty());
	ChatInputTextBox->SetVisibility(ESlateVisibility::Collapsed);

	if (MessageScrollBox && MessageScrollBox->GetChildrenCount() > 0)
	{
		RestartHideTimer();
	}
	else if (ChatHistoryPanel)
	{
		ChatHistoryPanel->SetVisibility(ESlateVisibility::Collapsed);
	}

}

void UGProjectChatWidget::HandleTextCommitted(const FText& Text, ETextCommit::Type CommitMehod)
{
	if (CommitMehod != ETextCommit::OnEnter)
	{
		return;
	}

	const FString Message = Text.ToString();

	AGProjectPlayerController* PC = Cast<AGProjectPlayerController>(GetOwningPlayer());
	if (!PC)
	{
		return;
	}

	PC->SendChatMessage(Message);
	PC->CloseChat();
}

FLinearColor UGProjectChatWidget::GetPlayerColor(int32 PlayerID) const
{
	static const TArray<FLinearColor> PlayerColors =
	{
		FLinearColor(0.3f, 0.65f, 1.0f),
		FLinearColor(1.0f, 0.55f, 0.20f),
		FLinearColor(0.35f, 0.90f, 0.45f),
		FLinearColor(0.90f, 0.40f, 0.85f)
	};
	
	const uint32 ColorIndex = GetTypeHash(PlayerID) % PlayerColors.Num();

	return PlayerColors[ColorIndex];
}

void UGProjectChatWidget::RestartHideTimer()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	World->GetTimerManager().SetTimer(
		HideChatTimerHandle,
		this,
		&ThisClass::HideChatHistory,
		ChatVisibleDuration,
		false
	);
}

void UGProjectChatWidget::HideChatHistory()
{
	if (bChatInputOpen)
	{
		return;
	}

	if (ChatHistoryPanel)
	{
		ChatHistoryPanel->SetVisibility(ESlateVisibility::Collapsed);
	}
}
