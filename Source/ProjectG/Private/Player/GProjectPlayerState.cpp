// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/GProjectPlayerState.h"

#include "AbilitySystem/GProjectAbilitySystemComponent.h"
#include "AbilitySystem/GProjectAttributeSet.h"
#include "Character/GProjectCharacter.h"
#include "Net/UnrealNetwork.h"

AGProjectPlayerState::AGProjectPlayerState()
{
	bReplicates = true;

	AbilitySystemComponent = CreateDefaultSubobject<UGProjectAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<UGProjectAttributeSet>(TEXT("AttributeSet"));

	SetNetUpdateFrequency(100.0f);
}

UAbilitySystemComponent* AGProjectPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UGProjectAbilitySystemComponent* AGProjectPlayerState::GetGProjectAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UGProjectAttributeSet* AGProjectPlayerState::GetAttributeSet() const
{
	return AttributeSet;
}

void AGProjectPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(
		AGProjectPlayerState,
		Team
	);

	DOREPLIFETIME(
		AGProjectPlayerState,
		PlayerColorIndex
	);

	DOREPLIFETIME(
		AGProjectPlayerState,
		PlayerName
	);

	DOREPLIFETIME(
		AGProjectPlayerState,
		PlayerLobbyStatus
	);

	DOREPLIFETIME(
		AGProjectPlayerState, 
		SlotIndex
	);
}

void AGProjectPlayerState::SetTeam(EGProjectTeam NewTeam)
{
	if (!HasAuthority() || Team == NewTeam) {
		return;
	}

	Team = NewTeam;
	
	OnTeamChanged.Broadcast(Team);

	ForceNetUpdate();
}

void AGProjectPlayerState::SetPlayerColorIndex(int32 NewColorIndex)
{
	if (!HasAuthority())
	{
		return;
	}

	PlayerColorIndex = NewColorIndex;
	OnRep_PlayerColorIndex();
}

void AGProjectPlayerState::OnRep_PlayerColorIndex()
{
	if (AGProjectCharacter* Character = Cast<AGProjectCharacter>(GetPawn()))
	{
		Character->ApplyPlayerColor(PlayerColorIndex);
	}
}

void AGProjectPlayerState::OnRep_Team()
{
	OnTeamChanged.Broadcast(Team);
}

void AGProjectPlayerState::CopyProperties(APlayerState* NewPlayerState)
{
	Super::CopyProperties(NewPlayerState);

	if (AGProjectPlayerState* NewPS = Cast<AGProjectPlayerState>(NewPlayerState))
	{
		NewPS->SetPlayerName(this->GetPlayerName());
	}
}

void AGProjectPlayerState::SetPlayerName(const FString& InName)
{
	if (!InName.IsEmpty())
	{
		Super::SetPlayerName(InName);

		PlayerName = InName;
	}
	else
	{
		PlayerName = Super::GetPlayerName();
	}

	OnPlayerNameChanged.Broadcast(PlayerName);
}

void AGProjectPlayerState::SetPlayerLobbyStatus(EGProjectPlayerLobbyStatus NewStatus)
{
	if (PlayerLobbyStatus == NewStatus)
	{
		return;
	}

	PlayerLobbyStatus = NewStatus;

	if (GEngine)
	{
		FString StateText;
		switch (PlayerLobbyStatus)
		{
		case EGProjectPlayerLobbyStatus::Wait:   StateText = TEXT("Wait"); break;
		case EGProjectPlayerLobbyStatus::Ready:  StateText = TEXT("Ready"); break;
		case EGProjectPlayerLobbyStatus::Master: StateText = TEXT("Master"); break;
		}
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("[%s] Player Lobby State: %s"), *GetPlayerName(), *StateText));
	}
	OnLobbyStatusChanged.Broadcast(PlayerLobbyStatus);
}

void AGProjectPlayerState::OnRep_PlayerLobbyStatus()
{
	OnLobbyStatusChanged.Broadcast(PlayerLobbyStatus);
}

void AGProjectPlayerState::SetSlotIndex(int32 NewIndex)
{
	if (HasAuthority() && SlotIndex != NewIndex)
	{
		SlotIndex = NewIndex;
	}
}

void AGProjectPlayerState::OnRep_SlotIndex()
{
	// Add Effect
}

void AGProjectPlayerState::OnRep_PlayerName()
{
	Super::OnRep_PlayerName();

	OnPlayerNameChanged.Broadcast(PlayerName);
}