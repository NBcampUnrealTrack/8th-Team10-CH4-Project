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
	void SetSlotInfo(const FString& InPlayerName, const FString& InStatus);

protected:
	virtual void NativePreConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> PlayerNameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> StatusText;
};
