// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "GProjectGameState.generated.h"

DECLARE_MULTICAST_DELEGATE(FGProjectPlayerListChangedSignature);

DECLARE_MULTICAST_DELEGATE_ThreeParams(
	FGProjectChatMessageReceivedSignature,
	int32,
	const FString&,
	const FString&
);

UCLASS()
class PROJECTG_API AGProjectGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	virtual void AddPlayerState(APlayerState* PlayerState) override;
	virtual void RemovePlayerState(APlayerState* PlayerState) override;

	void BroadcastChatMessage(int32 SenderPlayerID, const FString& SenderName, const FString& Message);

	FGProjectPlayerListChangedSignature OnPlayerListChanged;

	FGProjectChatMessageReceivedSignature OnChatMessageReceived;

private:
	UFUNCTION(NetMulticast, Reliable)
	void MulticastReceiveChatMessage(int32 SenderPlayerID, const FString& SenderName, const FString& Message);
};
