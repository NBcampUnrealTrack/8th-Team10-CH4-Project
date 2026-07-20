// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "Player/GProjectPlayerState.h"
#include "GProjectTeamPlayerStart.generated.h"

/** Player start reserved for a specific team and ordered slot. */
UCLASS()
class PROJECTG_API AGProjectTeamPlayerStart : public APlayerStart
{
	GENERATED_BODY()

public:
	AGProjectTeamPlayerStart(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	EGProjectTeam GetStartTeam() const { return StartTeam; }
	int32 GetSlotIndex() const { return SlotIndex; }

private:
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Team Start", meta = (AllowPrivateAccess = "true"))
	EGProjectTeam StartTeam = EGProjectTeam::None;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Team Start", meta = (AllowPrivateAccess = "true", ClampMin = "0"))
	int32 SlotIndex = 0;
};
