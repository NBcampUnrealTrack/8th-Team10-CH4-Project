// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/GProjectRoundTransitionWidget.h"

#include "Animation/WidgetAnimation.h"
#include "Components/TextBlock.h"

void UGProjectRoundTransitionWidget::ShowNextRound(int32 NextRound)
{
	SetVisibility(ESlateVisibility::HitTestInvisible);

	if (RoundText)
	{
		RoundText->SetText(FText::Format(NSLOCTEXT("RoundTransition", "NextRoundText", "Next Round {0}"), FText::AsNumber(NextRound)));
	}

	if (RoundTransitionAnimation)
	{
		StopAnimation(RoundTransitionAnimation);
		PlayAnimation(RoundTransitionAnimation);
	}
}

void UGProjectRoundTransitionWidget::HideTransition()
{
	SetVisibility(ESlateVisibility::Collapsed);
}
