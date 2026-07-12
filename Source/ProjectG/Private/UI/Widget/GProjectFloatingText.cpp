// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/Widget/GProjectFloatingText.h"

AGProjectFloatingText::AGProjectFloatingText()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;
	SetReplicatingMovement(false);
}
