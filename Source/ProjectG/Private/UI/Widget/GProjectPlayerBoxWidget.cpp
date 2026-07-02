// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/Widget/GProjectPlayerBoxWidget.h"

#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "UI/WidgetController/GProjectPlayerBoxWidgetController.h"

void UGProjectPlayerBoxWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	RefreshHealth();
	RefreshSP();
}

void UGProjectPlayerBoxWidget::NativeWidgetControllerSet()
{
	Super::NativeWidgetControllerSet();

	UGProjectPlayerBoxWidgetController* BoxController = Cast<UGProjectPlayerBoxWidgetController>(WidgetController);
	if (!BoxController)
	{
		return;
	}

	BoxController->OnHealthChanged.RemoveDynamic(this, &ThisClass::SetHealth);
	BoxController->OnHealthChanged.AddDynamic(this, &ThisClass::SetHealth);
	BoxController->OnMaxHealthChanged.RemoveDynamic(this, &ThisClass::SetMaxHealth);
	BoxController->OnMaxHealthChanged.AddDynamic(this, &ThisClass::SetMaxHealth);
	BoxController->OnSPChanged.RemoveDynamic(this, &ThisClass::SetSP);
	BoxController->OnSPChanged.AddDynamic(this, &ThisClass::SetSP);
	BoxController->OnMaxSPChanged.RemoveDynamic(this, &ThisClass::SetMaxSP);
	BoxController->OnMaxSPChanged.AddDynamic(this, &ThisClass::SetMaxSP);

	SetPlayerName(BoxController->GetPlayerName());
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
		const int32 HealthPercent = FMath::RoundToInt((Health / MaxHealth) * 100.0f);
		HPText->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), HealthPercent)));
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
