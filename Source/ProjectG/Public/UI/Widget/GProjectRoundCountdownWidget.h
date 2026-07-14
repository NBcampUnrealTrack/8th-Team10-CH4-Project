// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TimerManager.h"
#include "GProjectRoundCountdownWidget.generated.h"

class UTextBlock;

UCLASS()
class PROJECTG_API UGProjectRoundCountdownWidget
	: public UUserWidget
{
	GENERATED_BODY()

public:
	void ShowCountdown(int32 CountdownValue);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CountdownText;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Countdown",
		meta = (ClampMin = "0.1")
	)
	float FightDisplayDuration = 1.0f;

private:
	void HideCountdown();

	FTimerHandle HideTimerHandle;
};
