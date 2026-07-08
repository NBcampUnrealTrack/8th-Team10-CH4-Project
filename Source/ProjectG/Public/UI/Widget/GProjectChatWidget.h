// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Types/SlateEnums.h"
#include "UI/Widget/GProjectUserWidget.h"
#include "GProjectChatWidget.generated.h"

class UEditableTextBox;
class UScrollBox;
class UGProjectChatMessageWidget;
class UWidget;
/**
 * 
 */
UCLASS()
class PROJECTG_API UGProjectChatWidget : public UGProjectUserWidget
{
	GENERATED_BODY()

public:
	void AddChatMessage(int32 SenderPlayerID, const FString& SenderName, const FString& Message);

	void OpenChatInput();
	void CloseChatInput();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(EditDefaultsOnly, Category = "Chat")
	TSubclassOf<UGProjectChatMessageWidget> ChatMessageWidgetClass;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidget> ChatHistoryPanel;

	UPROPERTY(EditDefaultsOnly, Category = "Chat")
	float ChatVisibleDuration = 10.0f;

private:
	UFUNCTION()
	void HandleTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> MessageScrollBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> ChatInputTextBox;

	FLinearColor GetPlayerColor(int32 PlayerID) const;

	void RestartHideTimer();
	void HideChatHistory();

	FTimerHandle HideChatTimerHandle;

	bool bChatInputOpen = false;
	
};
