// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/GProjectMatchTimerWidget.h"

#include "Components/TextBlock.h"

void UGProjectMatchTimerWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	SetRemainTime(InitialRemainTime);
}

void UGProjectMatchTimerWidget::SetRemainTime(int32 Time)
{
	if (!TimerText)
	{
		return;
	}

	TimerText->SetText(FormatTime(Time));
}

FText UGProjectMatchTimerWidget::FormatTime(int32 Seconds) const
{
	const int32 Time = FMath::Max(Seconds, 0);

	return FText::FromString(FString::Printf(TEXT("%d"), Time));
}
