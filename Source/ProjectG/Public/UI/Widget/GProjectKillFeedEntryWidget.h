// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TimerManager.h"
#include "GProjectKillFeedEntryWidget.generated.h"

class UImage;
class UTextBlock;

UCLASS()
class PROJECTG_API UGProjectKillFeedEntryWidget
	: public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializeKillFeedEntry(
		const FString& KillerName,
		int32 KillerColorIndex,
		const FString& VictimName,
		int32 VictimColorIndex
	);

protected:
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> KillerNameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> KillIconImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> VictimNameText;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Kill Feed",
		meta = (ClampMin = "0.1")
	)
	float DisplayDuration = 3.0f;

private:
	void RemoveKillFeedEntry();

	FTimerHandle RemoveTimerHandle;
};