// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "GProjectGameState.generated.h"

class AGProjectPlayerState;
enum class EGProjectTeam : uint8;


UENUM(BlueprintType)
enum class ERoundPhase : uint8
{
	Waiting,
	Countdown,
	Playing,
	RoundResult,
	Intermission,
	Finished
};

UENUM(BlueprintType)
enum class ERoundResult : uint8
{
	None,
	RedWin,
	BlueWin,
	Draw
};

UENUM(BlueprintType)
enum class ERoundEndReason : uint8
{
	None,
	Elimination,
	TimeUp
};

USTRUCT(BlueprintType)
struct FGProjectRoundResultData
{
	GENERATED_BODY();

	UPROPERTY(BlueprintReadOnly)
	ERoundResult Result = ERoundResult::None;

	UPROPERTY(BlueprintReadOnly)
	ERoundEndReason Reason = ERoundEndReason::None;

	UPROPERTY(BlueprintReadOnly)
	float RedTeamTotalHP = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float BlueTeamTotalHP = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	int32 RedTeamRoundWins = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 BlueTeamRoundWins = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 Round = 0;
};

DECLARE_MULTICAST_DELEGATE_OneParam(
	FGProjectCurrentRoundChangedSignature,
	int32
);

DECLARE_MULTICAST_DELEGATE_OneParam(
	FGProjectRoundPhaseChangedSignature,
	ERoundPhase
);

DECLARE_MULTICAST_DELEGATE(FGProjectPlayerListChangedSignature);
DECLARE_MULTICAST_DELEGATE_OneParam(FGProjectMatchTimeChangedSignature, int32);

DECLARE_MULTICAST_DELEGATE_ThreeParams(
	FGProjectChatMessageReceivedSignature,
	int32,
	const FString&,
	const FString&
);

DECLARE_MULTICAST_DELEGATE_TwoParams(
	FGProjectTeamRoundWinsChangedSignature,
	int32,
	int32
);

DECLARE_MULTICAST_DELEGATE_SixParams(
	FGProjectKillFeedReceivedSignature,
	int32,
	const FString&,
	int32,
	int32,
	const FString&,
	int32
);

DECLARE_MULTICAST_DELEGATE_OneParam(
	FGProjectRoundCountdownChangedSignature,
	int32
);

DECLARE_MULTICAST_DELEGATE_OneParam(
	FGProjectRoundResultReceivedSignature,
	const FGProjectRoundResultData&
)

UCLASS()
class PROJECTG_API AGProjectGameState : public AGameState
{
	GENERATED_BODY()

public:
	virtual void AddPlayerState(APlayerState* PlayerState) override;
	virtual void RemovePlayerState(APlayerState* PlayerState) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	int32 GetRemainMatchTime() const { return RemainMatchTime; }
	void SetRemainMatchTime(int32 Time);

	void BroadcastChatMessage(int32 SenderPlayerID, const FString& SenderName, const FString& Message);

	void SetCurrentRound(int32 NewRound);
	int32 GetCurrentRound() const;

	void SetRoundPhase(ERoundPhase NewPhase);
	ERoundPhase GetRoundPhase() const;

	void AddTeamRoundWin(EGProjectTeam Winner);
	void ResetTeamRoundWins();

	void BroadcastKillFeed(AGProjectPlayerState* KillerPlayerState, AGProjectPlayerState* VictimPlayerState);

	void BroadcastRoundCountdown(int32 CountdownValue);

	void BroadcastRoundResult(const FGProjectRoundResultData& RoundResultData);

	const FGProjectRoundResultData& GetLastRoundResultData() const { return LastRoundResultData; }

	int32 GetRedTeamRoundWins() const { return RedTeamRoundWins; }
	int32 GetBlueTeamRoundWins() const { return BlueTeamRoundWins; }

	//int32 GetRoundCountdownValue() const { return RoundCountdownValue; }

	FGProjectMatchTimeChangedSignature OnMatchTimeChanged;

	FGProjectPlayerListChangedSignature OnPlayerListChanged;

	FGProjectChatMessageReceivedSignature OnChatMessageReceived;

	FGProjectCurrentRoundChangedSignature OnCurrentRoundChanged;
	FGProjectRoundPhaseChangedSignature OnRoundPhaseChanged;

	FGProjectTeamRoundWinsChangedSignature OnTeamRoundWinsChanged;

	FGProjectKillFeedReceivedSignature OnKillFeedReceived;

	FGProjectRoundCountdownChangedSignature OnRoundCountdownChanged;

	FGProjectRoundResultReceivedSignature OnRoundResultReceived;
	
	int32 GetRoundDuration() const { return RoundDuration; }
	void SetRoundDuration(int32 NewDuration);

private:
	UPROPERTY(ReplicatedUsing = OnRep_RemainMatchTime)
	int32 RemainMatchTime = 180;

	UFUNCTION()
	void OnRep_RemainMatchTime();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastReceiveChatMessage(int32 SenderPlayerID, const FString& SenderName, const FString& Message);

	UPROPERTY(ReplicatedUsing = OnRep_CurrentRound)
	int32 CurrentRound = 0;

	UPROPERTY(ReplicatedUsing = OnRep_RoundPhase)
	ERoundPhase RoundPhase = ERoundPhase::Waiting;

	UPROPERTY(ReplicatedUsing = OnRep_TeamRoundWins)
	int32 RedTeamRoundWins = 0;

	UPROPERTY(ReplicatedUsing = OnRep_TeamRoundWins)
	int32 BlueTeamRoundWins = 0;

	UPROPERTY(ReplicatedUsing = OnRep_LastRoundResultData)
	FGProjectRoundResultData LastRoundResultData;

	UFUNCTION()
	void OnRep_CurrentRound();

	UFUNCTION()
	void OnRep_RoundPhase();

	UFUNCTION()
	void OnRep_TeamRoundWins();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastReceiveKillFeed(
		int32 KillerPlayerId,
		const FString& KillerName,
		int32 KillerColorIndex,
		int32 VictimPlayerId,
		const FString& VictimName,
		int32 VictimColorIndex
	);

	UFUNCTION()
	void OnRep_LastRoundResultData();

	//UFUNCTION()
	//void OnRep_RoundCountdownValue();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastRoundCountdown(int32 CountdownValue);

	//UPROPERTY(ReplicatedUsing = OnRep_RoundCountdownValue)
	//int32 RoundCountdownValue = -1;
	
protected:

	UPROPERTY(Transient, ReplicatedUsing = OnRep_RoundDuration)
	int32 RoundDuration;

	UFUNCTION()
	void OnRep_RoundDuration();
};
