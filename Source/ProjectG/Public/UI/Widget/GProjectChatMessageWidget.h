// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/GProjectUserWidget.h"
#include "GProjectChatMessageWidget.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class PROJECTG_API UGProjectChatMessageWidget : public UGProjectUserWidget
{
	GENERATED_BODY()
	
public:
	void SetMessage(const FString& SenderName, const FString& Message, const FLinearColor& SenderColor);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SenderNameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> MessageText;
};
