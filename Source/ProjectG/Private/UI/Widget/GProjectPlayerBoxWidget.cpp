// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/Widget/GProjectPlayerBoxWidget.h"

#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UGProjectPlayerBoxWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	RefreshHealth();
	RefreshSP();
}

void UGProjectPlayerBoxWidget::SetPlayerName(const FText& NewName)
{
	if (NameText)
	{
		NameText->SetText(NewName);
	}
}

void UGProjectPlayerBoxWidget::SetCharacterImage(UTexture2D* NewImage)
{
	if (CharacterImage && NewImage)
	{
		CharacterImage->SetBrushFromTexture(NewImage);
	}
}

void UGProjectPlayerBoxWidget::SetHealth(float NewHealth)
{
	Health = NewHealth;
	RefreshHealth();
}

void UGProjectPlayerBoxWidget::SetMaxHealth(float NewMaxHealth)
{
	MaxHealth = FMath::Max(NewMaxHealth, 1.0f);
	RefreshHealth();
}

void UGProjectPlayerBoxWidget::SetSP(float NewSP)
{
	SP = NewSP;
	RefreshSP();
}

void UGProjectPlayerBoxWidget::SetMaxSP(float NewMaxSP)
{
	MaxSP = FMath::Max(NewMaxSP, 1.0f);
	RefreshSP();
}

void UGProjectPlayerBoxWidget::RefreshHealth()
{
	if (HPBar)
	{
		HPBar->SetPercent(FMath::Clamp(Health / MaxHealth, 0.0f, 1.0f));
	}

	if (HPText)
	{
		HPText->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), FMath::RoundToInt(Health), FMath::RoundToInt(MaxHealth))));
	}
}

void UGProjectPlayerBoxWidget::RefreshSP()
{
	if (SPBar)
	{
		SPBar->SetPercent(FMath::Clamp(SP / MaxSP, 0.0f, 1.0f));
	}

	if (SPText)
	{
		SPText->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), FMath::RoundToInt(SP), FMath::RoundToInt(MaxSP))));
	}
}
