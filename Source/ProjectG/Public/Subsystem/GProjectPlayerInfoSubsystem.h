// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GProjectPlayerInfoSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTG_API UGProjectPlayerInfoSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FString PlayerName;

	UPROPERTY()
	int32 Sound;

public:
	FString GetPlayerName() { return PlayerName; }
	void SetPlayerName(FString InName) { PlayerName = InName; }


};
