// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/GProjectUserWidget.h"
#include "GProjectLobbyWidget.generated.h"

class UTextBlock;
class UEditableTextBox;
class UButton;

UCLASS()
class PROJECTG_API UGProjectLobbyWidget : public UGProjectUserWidget
{
	GENERATED_BODY()
	
public:
	void UpdatePlayerCountText(int32 CurrentPlayers, int32 RequiredPlayers);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PlayerCountText;

};