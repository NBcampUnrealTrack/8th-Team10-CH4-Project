// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/GProjectUserWidget.h"
#include "GProjectLobbyStatusWidget.generated.h"

class UTextBlock;

UCLASS()
class PROJECTG_API UGProjectLobbyStatusWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void UpdateStatusText(const FString& StatusStr, const FLinearColor& Color);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> StatusText;
};