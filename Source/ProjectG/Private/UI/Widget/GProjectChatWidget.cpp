// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/GProjectChatWidget.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Components/EditableTextBox.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Player/GProjectPlayerController.h"

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
			&ThisClass::HandleCommitted
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
}

void UGProjectChatWidget::NativeDestruct()
{
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

	PC->UnregisterChatWidget(this);

	Super::NativeDestruct();
}

void UGProjectChatWidget::AddChatMessage(const FString& SenderName, const FString& Message)
{
	if (!MessageScrollBox || !WidgetTree)
	{
		return;
	}

	UTextBlock* MessageText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());

	if (!MessageText)
	{
		return;
	}

	const FString FormattedMessage = FString::Printf(TEXT("%s: %s"), *SenderName, *Message);

	MessageText->SetText(FText::FromString(FormattedMessage));
	MessageText->SetAutoWrapText(true);

	MessageScrollBox->AddChild(MessageText);

	while (MessageScrollBox->GetChildrenCount() > MaxVisibleChatMessage)
	{
		MessageScrollBox->RemoveChildAt(0);
	}

	MessageScrollBox->ScrollToEnd();
}

void UGProjectChatWidget::OpenChatInput()
{
	AGProjectPlayerController* PC = Cast<AGProjectPlayerController>(GetOwningPlayer());
	if (!PC || !ChatInputTextBox)
	{
		return;
	}

	ChatInputTextBox->SetText(FText::GetEmpty());
	ChatInputTextBox->SetVisibility(ESlateVisibility::Visible);

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
	if (!ChatInputTextBox)
	{
		return;	
	}

	ChatInputTextBox->SetText(FText::GetEmpty());
	ChatInputTextBox->SetVisibility(ESlateVisibility::Collapsed);
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
