// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/GProjectWidgetController.h"
#include "GProjectOverlayWidgetController.generated.h"

class AGProjectPlayerState;

enum class EGProjectTeam : uint8;
enum class ERoundPhase : uint8;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FGProjectOnPlayerListChangedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGProjectOnMatchTimeChangedSignature, int32, RemainTime);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FGProjectOnChatMessageReceivedSignature,
	int32, SenderPlayerID,
	const FString&, SenderName,
	const FString&, Message
);

DECLARE_MULTICAST_DELEGATE_TwoParams(
	FGProjectRoundPhaseUIChangedSignature,
	ERoundPhase,
	int32
);

DECLARE_MULTICAST_DELEGATE_TwoParams(
	FGProjectTeamScoreUIChangedSignature,
	int32,
	int32
);

DECLARE_MULTICAST_DELEGATE_SixParams(
	FGProjectKillFeedUIReceivedSignature,
	int32,
	const FString&,
	int32,
	int32,
	const FString&,
	int32
);

DECLARE_MULTICAST_DELEGATE_OneParam(
	FGProjectRoundCountdownUIChangedSignature,
	int32
);

UCLASS(BlueprintType, Blueprintable)
class PROJECTG_API UGProjectOverlayWidgetController : public UGProjectWidgetController
{
	GENERATED_BODY()

public:
	virtual void BindCallbacksToDependencies() override;
	
	virtual void BroadcastInitialValues() override;

	TArray<AGProjectPlayerState*> GetOrderedPlayerStates() const;

	UPROPERTY(BlueprintAssignable, Category = "Player List")
	FGProjectOnPlayerListChangedSignature OnPlayerListChanged;

	UPROPERTY(BlueprintAssignable, Category = "Match")
	FGProjectOnMatchTimeChangedSignature OnMatchTimeChanged;

	UPROPERTY(BlueprintAssignable, Category = "Chat")
	FGProjectOnChatMessageReceivedSignature OnChatMessageReceived;

	FGProjectRoundPhaseUIChangedSignature OnRoundPhaseUIChanged;

	FGProjectTeamScoreUIChangedSignature OnTeamScoreUIChanged;

	FGProjectKillFeedUIReceivedSignature OnKillFeedReceived;

	FGProjectRoundCountdownUIChangedSignature OnRoundCountdownChanged;

private:
	void HandlePlayerListChanged();
	void HandleMatchTimeChanged(int32 RemainTime);

	void HandleChatMessageReceived(int32 SenderPlayerID, const FString& SenderName, const FString& Message);

	void BindTeamCallbacks();
	void HandlePlayerTeamChanged(EGProjectTeam NewTeam);

	void HandleRoundPhaseChanged(ERoundPhase NewPhase);

	void HandleTeamRoundWinsChanged(int32 RedTeamWins, int32 BlueTeamWins);

	void HandleKillFeedReceived(
		int32 KillerPlayerId,
		const FString& KillerName,
		int32 KillerColorIndex,
		int32 VictimPlayerId,
		const FString& VictimName,
		int32 VictimColorIndex
	);

	void HandleRoundCountdownChanged(int32 CountdownValue);
};
