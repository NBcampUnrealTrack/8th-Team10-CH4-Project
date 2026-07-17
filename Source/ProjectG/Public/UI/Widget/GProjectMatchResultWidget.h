// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/GProjectUserWidget.h"
#include "GProjectMatchResultWidget.generated.h"

class UTextBlock;
class UButton;

UCLASS()
class PROJECTG_API UGProjectMatchResultWidget
	: public UGProjectUserWidget
{
	GENERATED_BODY()

public:
	void ShowResult(
		bool bVictory,
		int32 RedTeamWins,
		int32 BlueTeamWins
	);

	void HideResult();

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnReturnButtonClicked();

	UFUNCTION()
	void OnExitButtonClicked();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ResultText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> FinalScoreText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ReturnButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ExitButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> WaitingHostText;
};
