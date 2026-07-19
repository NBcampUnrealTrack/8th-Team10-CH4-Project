// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/GProjectTitleGameMode.h"

#include "Player/GProjectTitlePlayerController.h"

AGProjectTitleGameMode::AGProjectTitleGameMode()
{
	PlayerControllerClass = AGProjectTitlePlayerController::StaticClass();
	DefaultPawnClass = nullptr;
	HUDClass = nullptr;
	bStartPlayersAsSpectators = true;
}
