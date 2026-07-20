// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GProjectSessionRowWidget.generated.h"

class UButton;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionRowClicked, int32, SessionIndex);

UCLASS()
class PROJECTG_API UGProjectSessionRowWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;

	void SetupSessionRow(int32 InSessionIndex, const FString& InRoomName, int32 CurrentPlayers, int32 MaxPlayers);

	UPROPERTY(BlueprintAssignable)
	FOnSessionRowClicked OnSessionRowClicked;

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> RoomNameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PlayerCountText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> JoinButton;

	int32 SessionIndex = INDEX_NONE;

	UFUNCTION()
	void HandleJoinButtonClicked();
};
