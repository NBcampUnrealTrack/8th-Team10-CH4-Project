// Copyright Epic Games, Inc. All Rights Reserved.

#include "World/GProjectKillVolume.h"

#include "AbilitySystem/GProjectAbilitySystemComponent.h"
#include "AbilitySystem/GProjectAttributeSet.h"
#include "Character/GProjectCharacter.h"
#include "Game/GProjectGameState.h"
#include "Player/GProjectPlayerState.h"

AGProjectKillVolume::AGProjectKillVolume()
{
	OnActorBeginOverlap.AddDynamic(this, &ThisClass::HandleActorBeginOverlap);
}

void AGProjectKillVolume::HandleActorBeginOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	if (!HasAuthority())
	{
		return;
	}

	AGProjectCharacter* Character = Cast<AGProjectCharacter>(OtherActor);
	if (!Character || Character->IsDead())
	{
		return;
	}

	if (UGProjectAbilitySystemComponent* ASC = Character->GetGProjectAbilitySystemComponent())
	{
		ASC->SetNumericAttributeBase(UGProjectAttributeSet::GetHealthAttribute(), 0.0f);
	}

	if (AGProjectGameState* GameState = GetWorld() ? GetWorld()->GetGameState<AGProjectGameState>() : nullptr)
	{
		GameState->BroadcastKillFeed(nullptr, Character->GetPlayerState<AGProjectPlayerState>());
	}

	Character->HandleDeath();
}
