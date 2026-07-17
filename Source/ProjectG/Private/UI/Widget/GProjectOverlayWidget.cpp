// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/Widget/GProjectOverlayWidget.h"

#include "AbilitySystem/GProjectAbilitySystemComponent.h"
#include "AbilitySystem/GProjectAttributeSet.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Character/GProjectCharacter.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"
#include "Player/GProjectPlayerState.h"
#include "Targeting/GProjectLockOnComponent.h"
#include "UI/Widget/GProjectPlayerBoxWidget.h"
#include "UI/WidgetController/GProjectOverlayWidgetController.h"
#include "UI/WidgetController/GProjectPlayerBoxWidgetController.h"
#include "UI/WidgetController/GProjectWidgetController.h"
#include "UI/Widget/GProjectMatchTimerWidget.h"
#include "UI/Widget/GProjectChatWidget.h"
#include "Game/GProjectGameState.h"
#include "UI/Widget/GProjectRoundTransitionWidget.h"
#include "UI/Widget/GProjectMatchResultWidget.h"
#include "UI/Widget/GProjectMatchHeaderWidget.h"
#include "UI/Widget/GProjectKillFeedWidget.h"
#include "UI/Widget/GProjectRoundCountdownWidget.h"
#include "UI/Widget/GProjectRoundResultWidget.h"

void UGProjectOverlayWidget::NativeWidgetControllerSet()
{
	Super::NativeWidgetControllerSet();

	UGProjectOverlayWidgetController* OverlayController = Cast<UGProjectOverlayWidgetController>(WidgetController);
	if (!OverlayController)
	{
		return;
	}

	OverlayController->OnPlayerListChanged.RemoveDynamic(this, &ThisClass::RefreshPlayerBoxes);

	OverlayController->OnPlayerListChanged.AddDynamic(this, &ThisClass::RefreshPlayerBoxes);
	RefreshPlayerBoxes();

	OverlayController->OnMatchTimeChanged.RemoveDynamic(this, &ThisClass::HandleRemainTimeChanged);
	OverlayController->OnMatchTimeChanged.AddDynamic(this, &ThisClass::HandleRemainTimeChanged);

	BindLockOnComponent();

	OverlayController->OnChatMessageReceived.RemoveDynamic(this, &ThisClass::RefreshChatMessage);
	OverlayController->OnChatMessageReceived.AddDynamic(this, &ThisClass::RefreshChatMessage);

	OverlayController->OnRoundPhaseUIChanged.RemoveAll(this);
	OverlayController->OnRoundPhaseUIChanged.AddUObject(this, &ThisClass::HandleRoundPhaseUIChanged);

	OverlayController->OnTeamScoreUIChanged.RemoveAll(this);
	OverlayController->OnTeamScoreUIChanged.AddUObject(this, &ThisClass::HandleTeamScoreUIChanged);

	OverlayController->OnKillFeedReceived.RemoveAll(this);
	OverlayController->OnKillFeedReceived.AddUObject(this, &ThisClass::HandleKillFeedReceived);

	OverlayController->OnRoundCountdownChanged.RemoveAll(this);
	OverlayController->OnRoundCountdownChanged.AddUObject(this, &ThisClass::HandleRoundCountdownChanged);

	OverlayController->OnRoundResultUIReceived.RemoveAll(this);
	OverlayController->OnRoundResultUIReceived.AddUObject(this, &ThisClass::HandleRoundResultUIReceived);
}

void UGProjectOverlayWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	UpdateLockOnIndicator();
}

void UGProjectOverlayWidget::NativeDestruct()
{
	if (UGProjectOverlayWidgetController*
		OverlayController = Cast<UGProjectOverlayWidgetController>(WidgetController))
	{
		OverlayController->OnKillFeedReceived.RemoveAll(this);

		OverlayController->OnRoundCountdownChanged.RemoveAll(this);

		OverlayController->OnRoundResultUIReceived.RemoveAll(this);
	}

	PlayerBoxesByPlayerId.Reset();

	if (BoundLockOnComponent)
	{
		BoundLockOnComponent->OnLockOnTargetChanged.RemoveDynamic(
			this, &ThisClass::OnLockOnTargetChanged);
	}

	Super::NativeDestruct();

}

void UGProjectOverlayWidget::RefreshPlayerBoxes()
{

	UWorld* World = GetWorld();
	if (!World || World->bIsTearingDown)
	{
		return;
	}

	UGProjectOverlayWidgetController* OverlayController = Cast<UGProjectOverlayWidgetController>(WidgetController);
	if (!OverlayController || !RedTeamContainer || !BlueTeamContainer || !PlayerBoxWidgetClass)
	{
		return;
	}
	RedTeamContainer->ClearChildren();
	BlueTeamContainer->ClearChildren();

	PlayerBoxesByPlayerId.Reset();
	PlayerBoxControllers.Reset();

	for (AGProjectPlayerState* CurrentPlayerState : OverlayController->GetOrderedPlayerStates())
	{
		if (!CurrentPlayerState || !CurrentPlayerState->GetGProjectAbilitySystemComponent() || !CurrentPlayerState->GetAttributeSet())
		{
			continue;
		}
		UPanelWidget* TargetContainer = nullptr;

		switch (CurrentPlayerState->GetTeam())
		{
		case EGProjectTeam::Red:
			TargetContainer = RedTeamContainer;
			break;

		case EGProjectTeam::Blue:
			TargetContainer = BlueTeamContainer;
			break;

		case EGProjectTeam::None:
		default:
			continue;
		}
		
		UGProjectPlayerBoxWidgetController* BoxController = NewObject<UGProjectPlayerBoxWidgetController>(this);
		const FGProjectWidgetControllerParams Params(
			GetOwningPlayer(),
			CurrentPlayerState,
			CurrentPlayerState->GetGProjectAbilitySystemComponent(),
			CurrentPlayerState->GetAttributeSet());
		BoxController->SetWidgetControllerParams(Params);
		BoxController->BindCallbacksToDependencies();

		UGProjectPlayerBoxWidget* PlayerBox = CreateWidget<UGProjectPlayerBoxWidget>(GetOwningPlayer(), PlayerBoxWidgetClass);
		if (!PlayerBox)
		{
			continue;
		}

		PlayerBox->SetWidgetController(BoxController);

		PlayerBox->SetupPortrait(CurrentPlayerState);

		PlayerBox->ApplyTeamStyle(CurrentPlayerState->GetTeam());

		PlayerBoxesByPlayerId.Add(CurrentPlayerState->GetPlayerId(),PlayerBox);

		switch (CurrentPlayerState->GetTeam())
		{
		case EGProjectTeam::Red:
			RedTeamContainer->InsertChildAt(0, PlayerBox);
			break;

		case EGProjectTeam::Blue:
			BlueTeamContainer->AddChild(PlayerBox);
			break;

		case EGProjectTeam::None:
		default:
			break;
		}

		BoxController->BroadcastInitialValues();
		PlayerBoxControllers.Add(BoxController);
	}
}

void UGProjectOverlayWidget::BindLockOnComponent()
{
	if (BoundLockOnComponent)
	{
		BoundLockOnComponent->OnLockOnTargetChanged.RemoveDynamic(
			this, &ThisClass::OnLockOnTargetChanged);
	}

	const AGProjectCharacter* Character = Cast<AGProjectCharacter>(GetOwningPlayerPawn());
	BoundLockOnComponent = Character ? Character->GetLockOnComponent() : nullptr;
	if (!BoundLockOnComponent)
	{
		OnLockOnTargetChanged(nullptr);
		return;
	}

	BoundLockOnComponent->OnLockOnTargetChanged.AddDynamic(
		this, &ThisClass::OnLockOnTargetChanged);
	OnLockOnTargetChanged(BoundLockOnComponent->GetCurrentTarget());
}

void UGProjectOverlayWidget::OnLockOnTargetChanged(AActor* NewTarget)
{
	LockOnTarget = NewTarget;

	if (LockOnIndicator)
	{
		LockOnIndicator->SetVisibility(
			LockOnTarget ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
}

void UGProjectOverlayWidget::RefreshChatMessage(int32 SenderPlayerID, const FString& SenderName, const FString& Message)
{
	if (!ChatWidget)
	{
		return;
	}

	ChatWidget->AddChatMessage(SenderPlayerID, SenderName, Message);
}

void UGProjectOverlayWidget::UpdateLockOnIndicator()
{
	if (!LockOnIndicator)
	{
		return;
	}

	if (!IsValid(LockOnTarget))
	{
		LockOnIndicator->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	FVector2D WidgetPosition;
	const FVector WorldPosition = LockOnTarget->GetActorLocation() + FVector::UpVector * IndicatorHeightOffset;
	const bool bProjected = UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(
		GetOwningPlayer(),
		WorldPosition,
		WidgetPosition,
		true);

	UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(LockOnIndicator->Slot);
	if (!bProjected || !CanvasSlot)
	{
		LockOnIndicator->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	CanvasSlot->SetPosition(WidgetPosition);
	CanvasSlot->SetAlignment(FVector2D(0.5f));
	LockOnIndicator->SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UGProjectOverlayWidget::HandleRoundPhaseUIChanged(ERoundPhase NewPhase,int32 CurrentRound)
{
	if (RoundResultWidget && NewPhase != ERoundPhase::RoundResult)
	{
		RoundResultWidget->HideRoundResult();
	}

	switch (NewPhase)
	{
	case ERoundPhase::Intermission:
	{
		if (MatchResultWidget)
		{
			MatchResultWidget->HideResult();
		}

		if (RoundTransitionWidget)
		{
			RoundTransitionWidget->ShowNextRound(
				CurrentRound + 1
			);
		}

		break;
	}

	case ERoundPhase::Finished:
	{
		if (RoundTransitionWidget)
		{
			RoundTransitionWidget->HideTransition();
		}

		if (!MatchResultWidget ||
			!GetOwningPlayer())
		{
			break;
		}

		AGProjectGameState* GameState =
			GetOwningPlayer()
			->GetWorld()
			->GetGameState<AGProjectGameState>();

		AGProjectPlayerState* LocalPlayerState =
			GetOwningPlayer()
			->GetPlayerState<AGProjectPlayerState>();

		if (!GameState || !LocalPlayerState)
		{
			break;
		}

		const int32 RedTeamWins =
			GameState->GetRedTeamRoundWins();

		const int32 BlueTeamWins =
			GameState->GetBlueTeamRoundWins();

		const EGProjectTeam WinningTeam =
			RedTeamWins > BlueTeamWins
			? EGProjectTeam::Red
			: EGProjectTeam::Blue;

		const bool bLocalPlayerWon =
			LocalPlayerState->GetTeam() ==
			WinningTeam;

		MatchResultWidget->ShowResult(
			bLocalPlayerWon,
			RedTeamWins,
			BlueTeamWins
		);

		break;
	}

	case ERoundPhase::Waiting:
	case ERoundPhase::Playing:
	default:
	{
		if (RoundTransitionWidget)
		{
			RoundTransitionWidget->HideTransition();
		}

		if (MatchResultWidget)
		{
			MatchResultWidget->HideResult();
		}

		break;
	}
	}
}

void UGProjectOverlayWidget::HandleTeamScoreUIChanged(
	int32 RedTeamWins,
	int32 BlueTeamWins)
{
	if (!MatchHeaderWidget)
	{
		return;
	}

	MatchHeaderWidget->SetTeamScore(
		RedTeamWins,
		BlueTeamWins
	);
}

void UGProjectOverlayWidget::HandleRemainTimeChanged(
	int32 RemainTime)
{
	if (!MatchHeaderWidget)
	{
		return;
	}

	MatchHeaderWidget->SetRemainTime(
		RemainTime
	);
}

void UGProjectOverlayWidget::HandleKillFeedReceived(
	const int32 KillerPlayerId,
	const FString& KillerName,
	const int32 KillerColorIndex,
	const int32 VictimPlayerId,
	const FString& VictimName,
	const int32 VictimColorIndex)
{
	(void)KillerPlayerId;

	if (TObjectPtr<UGProjectPlayerBoxWidget>*
		FoundPlayerBox = PlayerBoxesByPlayerId.Find(VictimPlayerId))
	{
		UGProjectPlayerBoxWidget* PlayerBox = FoundPlayerBox->Get();

		if (IsValid(PlayerBox))
		{
			PlayerBox->SetDeathMarkVisible(true);
		}
	}
	
	if (KillFeedWidget)
	{
		KillFeedWidget->AddKillFeedEntry(
			KillerName,
			KillerColorIndex,
			VictimName,
			VictimColorIndex
		);
	}
}

void UGProjectOverlayWidget::HandleRoundCountdownChanged(const int32 CountdownValue)
{
	if (!RoundCountdownWidget)
	{
		return;
	}

	RoundCountdownWidget->ShowCountdown(CountdownValue);
}

void UGProjectOverlayWidget::HandleRoundResultUIReceived(const FGProjectRoundResultData& RoundResultData)
{
	if (!RoundResultWidget)
	{
		return;
	}

	RoundResultWidget->ShowRoundResult(RoundResultData);
}