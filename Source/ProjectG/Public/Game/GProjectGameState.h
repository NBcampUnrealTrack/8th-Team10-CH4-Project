// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "GProjectGameState.generated.h"

DECLARE_MULTICAST_DELEGATE(FGProjectPlayerListChangedSignature);

UCLASS()
class PROJECTG_API AGProjectGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	virtual void AddPlayerState(APlayerState* PlayerState) override;
	virtual void RemovePlayerState(APlayerState* PlayerState) override;

	FGProjectPlayerListChangedSignature OnPlayerListChanged;
};
