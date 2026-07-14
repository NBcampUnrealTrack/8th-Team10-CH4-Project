// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "GProjectLobbyGameState.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTG_API AGProjectLobbyGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	bool CanStartGame() const;
};
