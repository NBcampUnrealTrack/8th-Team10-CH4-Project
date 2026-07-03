// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/GProjectUserWidget.h"
#include "GProjectMatchTimerWidget.generated.h"

class UTextBlock;

/**
 * 
 */

UCLASS()
class PROJECTG_API UGProjectMatchTimerWidget : public UGProjectUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Match Timer")
	void SetRemainTime(int32 Time);

private:
	virtual void NativePreConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TimerText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Match Timer")
	int32 InitialRemainTime = 180;

	FText FormatTime(int32 Seconds) const;
};
