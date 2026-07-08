// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/GProjectUserWidget.h"
#include "GProjectMatchHeaderWidget.generated.h"

class UTextBlock;

UCLASS()
class PROJECTG_API UGProjectMatchHeaderWidget
	: public UGProjectUserWidget
{
	GENERATED_BODY()

public:
	void SetTeamScore(
		int32 RedTeamWins,
		int32 BlueTeamWins
	);

	void SetRemainTime(int32 RemainTime);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> RedScoreText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> BlueScoreText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TimerText;
};