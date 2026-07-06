// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "GProjectGameState.generated.h"

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

	FGProjectMatchTimeChangedSignature OnMatchTimeChanged;

	FGProjectPlayerListChangedSignature OnPlayerListChanged;

	FGProjectChatMessageReceivedSignature OnChatMessageReceived;

	FGProjectCurrentRoundChangedSignature OnCurrentRoundChanged;
	FGProjectRoundPhaseChangedSignature OnRoundPhaseChanged;

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

	UFUNCTION()
	void OnRep_CurrentRound();

	UFUNCTION()
	void OnRep_RoundPhase();
};
