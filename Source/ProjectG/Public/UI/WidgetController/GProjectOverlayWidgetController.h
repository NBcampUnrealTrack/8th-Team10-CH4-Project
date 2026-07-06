// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/GProjectWidgetController.h"
#include "GProjectOverlayWidgetController.generated.h"

class AGProjectPlayerState;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FGProjectOnPlayerListChangedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGProjectOnMatchTimeChangedSignature, int32, RemainTime);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FGProjectOnChatMessageReceivedSignature,
	int32, SenderPlayerID,
	const FString&, SenderName,
	const FString&, Message
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

private:
	void HandlePlayerListChanged();
	void HandleMatchTimeChanged(int32 RemainTime);

	UPROPERTY(BlueprintAssignable, Category = "Chat")
	FGProjectOnChatMessageReceivedSignature OnChatMessageReceived;

private:
	void HandlePlayerListChanged();

	void HandleChatMessageReceived(int32 SenderPlayerID, const FString& SenderName, const FString& Message);

};
