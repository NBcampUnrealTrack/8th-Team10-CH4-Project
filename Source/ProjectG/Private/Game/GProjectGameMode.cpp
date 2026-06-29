// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/GProjectGameMode.h"

#include "Character/GProjectCharacter.h"
#include "Game/GProjectGameState.h"
#include "Player/GProjectPlayerController.h"
#include "Player/GProjectPlayerState.h"
#include "UI/HUD/GProjectHUD.h"

AGProjectGameMode::AGProjectGameMode()
{
	DefaultPawnClass = AGProjectCharacter::StaticClass();
	PlayerControllerClass = AGProjectPlayerController::StaticClass();
	PlayerStateClass = AGProjectPlayerState::StaticClass();
	GameStateClass = AGProjectGameState::StaticClass();
	HUDClass = AGProjectHUD::StaticClass();
}
