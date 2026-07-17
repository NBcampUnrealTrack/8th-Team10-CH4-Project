// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/GProjectUserWidget.h"
#include "GProjectOverlayWidget.generated.h"

class AActor;
class UGProjectLockOnComponent;
class UGProjectOverlayWidgetController;
class UGProjectPlayerBoxWidget;
class UGProjectPlayerBoxWidgetController;
class UGProjectChatWidget;
class UGProjectRoundTransitionWidget;
class UGProjectMatchResultWidget;
class UGProjectMatchHeaderWidget;
class UImage;
class UPanelWidget;
class UGProjectMatchTimerWidget;
class UVerticalBox;
class UHorizontalBox;
class UGProjectKillFeedWidget;
class UGProjectRoundCountdownWidget;
class UGProjectRoundResultWidget;
struct FGProjectRoundResultData;

enum class ERoundPhase : uint8;


UCLASS()
class PROJECTG_API UGProjectOverlayWidget : public UGProjectUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeWidgetControllerSet() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> RedTeamContainer;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> BlueTeamContainer;

	UPROPERTY(EditDefaultsOnly, Category = "Player List")
	TSubclassOf<UGProjectPlayerBoxWidget> PlayerBoxWidgetClass;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UGProjectMatchTimerWidget> MatchTimerWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> LockOnIndicator;

	UPROPERTY(EditDefaultsOnly, Category = "Lock On")
	float IndicatorHeightOffset = 120.0f;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UGProjectChatWidget> ChatWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UGProjectRoundTransitionWidget> RoundTransitionWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UGProjectMatchResultWidget> MatchResultWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UGProjectMatchHeaderWidget> MatchHeaderWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UGProjectKillFeedWidget> KillFeedWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UGProjectRoundCountdownWidget> RoundCountdownWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UGProjectRoundResultWidget> RoundResultWidget;

private:
	UFUNCTION()
	void RefreshPlayerBoxes();
	
	UFUNCTION()
	void OnLockOnTargetChanged(AActor* NewTarget);

	UFUNCTION()
	void RefreshChatMessage(int32 SenderPlayerID, const FString& SenderName, const FString& Message);

	void BindLockOnComponent();
	void UpdateLockOnIndicator();

	void HandleRoundPhaseUIChanged(ERoundPhase NewPhase, int32 CurrentRound);

	void HandleTeamScoreUIChanged(int32 RedTeamWins, int32 BlueTeamWins);

	void HandleKillFeedReceived(
		int32 KillerPlayerId,
		const FString& KillerName,
		int32 KillerColorIndex,
		int32 VictimPlayerId,
		const FString& VictimName,
		int32 VictimColorIndex
	);

	void HandleRoundCountdownChanged(const int32 CountdownValue);

	void HandleRoundResultUIReceived(const FGProjectRoundResultData& RoundResultData);

	UFUNCTION()
	void HandleRemainTimeChanged(int32 RemainTime);

	UPROPERTY()
	TArray<TObjectPtr<UGProjectPlayerBoxWidgetController>> PlayerBoxControllers;

	UPROPERTY()
	TObjectPtr<UGProjectLockOnComponent> BoundLockOnComponent;

	UPROPERTY()
	TObjectPtr<AActor> LockOnTarget;

	UPROPERTY(Transient)
	TMap<int32, TObjectPtr<UGProjectPlayerBoxWidget>> PlayerBoxesByPlayerId;

};
