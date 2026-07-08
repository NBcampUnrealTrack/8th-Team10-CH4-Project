// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "GProjectKillVolume.generated.h"

UCLASS()
class PROJECTG_API AGProjectKillVolume : public ATriggerBox
{
	GENERATED_BODY()

public:
	AGProjectKillVolume();

private:
	UFUNCTION()
	void HandleActorBeginOverlap(AActor* OverlappedActor, AActor* OtherActor);
};
