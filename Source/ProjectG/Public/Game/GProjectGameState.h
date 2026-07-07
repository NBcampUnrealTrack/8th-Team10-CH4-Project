// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "GProjectGameState.generated.h"

enum class EGProjectTeam : uint8;

UENUM(BlueprintType)
enum class ERoundPhase : uint8
{
	Waiting,
	Playing,
	Intermission
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

	int32 GetRedTeamRoundWins() const { return RedTeamRoundWins; }
	int32 GetBlueTeamRoundWins() const { return BlueTeamRoundWins; }

	FGProjectMatchTimeChangedSignature OnMatchTimeChanged;

	FGProjectPlayerListChangedSignature OnPlayerListChanged;

	FGProjectChatMessageReceivedSignature OnChatMessageReceived;

	FGProjectCurrentRoundChangedSignature OnCurrentRoundChanged;
	FGProjectRoundPhaseChangedSignature OnRoundPhaseChanged;

	FGProjectTeamRoundWinsChangedSignature OnTeamRoundWinsChanged;

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

	UFUNCTION()
	void OnRep_CurrentRound();

	UFUNCTION()
	void OnRep_RoundPhase();

	UFUNCTION()
	void OnRep_TeamRoundWins();
};
