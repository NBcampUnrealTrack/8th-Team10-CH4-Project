// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/GProjectUserWidget.h"
#include "GProjectRoundTransitionWidget.generated.h"

class UTextBlock;
class UWidgetAnimation;
/**
 * 
 */
UCLASS()
class PROJECTG_API UGProjectRoundTransitionWidget : public UGProjectUserWidget
{
	GENERATED_BODY()

public:
	void ShowNextRound(int32 NextRound);
	void HideTransition();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> RoundText;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> RoundTransitionAnimation;
	
};
