// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/GProjectUserWidget.h"
#include "Player/GProjectPlayerState.h"
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

	void InitLobbyWidget(class AGProjectLobbyPlayerController* InPC);
	void RefreshButtonState(EGProjectPlayerLobbyStatus NewStatus = EGProjectPlayerLobbyStatus::Wait);

protected:

	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PlayerCountText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> LobbyActionButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> LobbyActionText;

private:
	UPROPERTY()
	TObjectPtr<class AGProjectLobbyPlayerController> OwningLobbyPC;

	UFUNCTION()
	void OnLobbyActionClicked();

};