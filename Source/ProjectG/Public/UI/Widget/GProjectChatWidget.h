// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Types/SlateEnums.h"
#include "UI/Widget/GProjectUserWidget.h"
#include "GProjectChatWidget.generated.h"

class UEditableTextBox;
class UScrollBox;
/**
 * 
 */
UCLASS()
class PROJECTG_API UGProjectChatWidget : public UGProjectUserWidget
{
	GENERATED_BODY()

public:
	void AddChatMessage(const FString& SenderName, const FString& Message);

	void OpenChatInput();
	void CloseChatInput();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UFUNCTION()
	void HandleTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> MessageScrollBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> ChatInputTextBox;
	
};
