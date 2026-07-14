// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/Widget/GProjectPlayerBoxWidget.h"

#include "Player/GProjectPlayerState.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "UI/WidgetController/GProjectPlayerBoxWidgetController.h"
#include "Character/GProjectCharacter.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/World.h"
#include "Actor/Portrait/GProjectPortraitActor.h"
#include "TimerManager.h"

void UGProjectPlayerBoxWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	SetDeathMarkVisible(false);
	RefreshHealth();
	RefreshSP();
	
	if (HPBar_Yellow && HPBar)
	{
		HPBar_Yellow->SetPercent(HPBar->GetPercent());
	}
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

void UGProjectPlayerBoxWidget::NativeDestruct()
{
	UWorld* World = GetWorld();

	if (World)
	{
		World->GetTimerManager().ClearTimer(
			PortraitRetryTimerHandle
		);
	}

	if (PortraitImage)
	{
		PortraitImage->SetBrushResourceObject(nullptr);
	}

	if (IsValid(PortraitActor))
	{
		PortraitActor->Destroy();
		PortraitActor = nullptr;
	}

	PortraitPlayerState.Reset();

	Super::NativeDestruct();
}

void UGProjectPlayerBoxWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	if (!HPBar_Yellow || !HPBar)
	{
		return;
	}
	
	if (HPDelayTimer > 0.0f)
	{
		HPDelayTimer -= InDeltaTime;
	}
	
	else
	{
		const float CurrentYellowPercent = HPBar_Yellow->GetPercent();
		const float TargetPercent = HPBar->GetPercent();
		
		if (FMath::IsNearlyEqual(CurrentYellowPercent, TargetPercent, 0.001f))
		{
			HPBar_Yellow->SetPercent(TargetPercent);
		}
		else
		{
			const float NewPercent = FMath::FInterpTo(CurrentYellowPercent, TargetPercent, InDeltaTime, HPInterpSpeed);
			HPBar_Yellow->SetPercent(NewPercent);
		}
	}
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

	if (Health > KINDA_SMALL_NUMBER)
	{
		SetDeathMarkVisible(Health <= KINDA_SMALL_NUMBER);
	}
	
	HPDelayTimer = HPDelayTime;
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

void UGProjectPlayerBoxWidget::ApplyTeamStyle(EGProjectTeam NewTeam)
{
	if (!PlayerFrame)
	{
		return;
	}

	switch (NewTeam)
	{
	case EGProjectTeam::Red:
		PlayerFrame->SetBrushColor(FLinearColor::FromSRGBColor(FColor(255, 110, 120, 255)));
		break;

	case EGProjectTeam::Blue:
		PlayerFrame->SetBrushColor(FLinearColor::FromSRGBColor(FColor(120, 190, 255, 255)));
		break;

	default:
		break;
	}
}

void UGProjectPlayerBoxWidget::SetupPortrait(
	AGProjectPlayerState* InPlayerState)
{
	PortraitPlayerState = InPlayerState;

	TrySetupPortrait();

	if (!IsValid(PortraitActor) && GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(
			PortraitRetryTimerHandle,
			this,
			&ThisClass::TrySetupPortrait,
			0.1f,
			true
		);
	}
}

void UGProjectPlayerBoxWidget::SetDeathMarkVisible(const bool bVisible)
{
	if (!DeathMark)
	{
		return;
	}

	DeathMark->SetVisibility(
		bVisible
		? ESlateVisibility::HitTestInvisible
		: ESlateVisibility::Collapsed
	);
}

void UGProjectPlayerBoxWidget::TrySetupPortrait()
{
	UWorld* World = GetWorld();

	if (!World || World->bIsTearingDown)
	{
		return;
	}

	if (IsValid(PortraitActor))
	{
		World->GetTimerManager().ClearTimer(
			PortraitRetryTimerHandle
		);

		return;
	}

	AGProjectPlayerState* PlayerState =
		PortraitPlayerState.Get();

	if (!IsValid(PlayerState))
	{
		return;
	}

	AGProjectCharacter* Character =
		Cast<AGProjectCharacter>(
			PlayerState->GetPawn()
		);

	const int32 PlayerColorIndex =
		PlayerState->GetPlayerColorIndex();

	if (!IsValid(Character) ||
		PlayerColorIndex == INDEX_NONE)
	{
		return;
	}

	if (!PortraitActorClass || !PortraitImage)
	{
		return;
	}

	FActorSpawnParameters SpawnParameters;

	SpawnParameters.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	SpawnParameters.ObjectFlags |= RF_Transient;

	const FVector PortraitLocation(
		100000.0f,
		PlayerColorIndex * 2000.0f,
		5000.0f
	);

	PortraitActor =
		World->SpawnActor<AGProjectPortraitActor>(
			PortraitActorClass,
			PortraitLocation,
			FRotator::ZeroRotator,
			SpawnParameters
		);

	if (!IsValid(PortraitActor))
	{
		return;
	}

	PortraitActor->InitializePortrait(
		Character,
		PlayerColorIndex
	);

	UTextureRenderTarget2D* RenderTarget =
		PortraitActor->GetRenderTarget();

	if (!RenderTarget)
	{
		PortraitActor->Destroy();
		PortraitActor = nullptr;
		return;
	}

	PortraitImage->SetBrushResourceObject(
		RenderTarget
	);

	World->GetTimerManager().ClearTimer(
		PortraitRetryTimerHandle
	);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[Portrait] Setup succeeded: Player=%s, Pawn=%s, Color=%d"),
		*PlayerState->GetPlayerName(),
		*GetNameSafe(Character),
		PlayerColorIndex
	);
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
		HPText->SetText(FText::FromString(FString::Printf(TEXT("%d"), HealthPercent)));
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
