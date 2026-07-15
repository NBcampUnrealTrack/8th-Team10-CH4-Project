// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/PlayerStartSlot/GProjectLobbyPlayerStartSlot.h"

#include "UI/Widget/Lobby/GProjectLobbyStatusWidget.h"
#include "Player/GProjectPlayerState.h"
#include "Components/WidgetComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"

AGProjectLobbyPlayerStartSlot::AGProjectLobbyPlayerStartSlot()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	SpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("SpawnPoint"));
	SpawnPoint->SetupAttachment(RootComponent);

	SlotInfoWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("SlotInfoWidget"));
	SlotInfoWidgetComponent->SetupAttachment(RootComponent);
	SlotInfoWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
	SlotInfoWidgetComponent->SetDrawSize(FVector2D(400.f, 150.f));
}

void AGProjectLobbyPlayerStartSlot::BeginPlay()
{
	Super::BeginPlay();

	RefreshSlotUI();
}

void AGProjectLobbyPlayerStartSlot::LinkPlayer(AGProjectPlayerState* InPlayerState)
{
	LinkedPlayerState = InPlayerState;

	if (LinkedPlayerState)
	{
		LinkedPlayerState->OnLobbyStatusChanged.RemoveAll(this);
		LinkedPlayerState->OnLobbyStatusChanged.AddUObject(this, &AGProjectLobbyPlayerStartSlot::RefreshSlotUI);
	}

	RefreshSlotUI();
}

void AGProjectLobbyPlayerStartSlot::UnlinkPlayer()
{
	if (LinkedPlayerState)
	{
		LinkedPlayerState->OnLobbyStatusChanged.RemoveAll(this);
		LinkedPlayerState = nullptr;
	}
	RefreshSlotUI();
}

void AGProjectLobbyPlayerStartSlot::RefreshSlotUI(EGProjectPlayerLobbyStatus NewStatus)
{
	UGProjectLobbyStatusWidget* SlotWidget = Cast<UGProjectLobbyStatusWidget>(SlotInfoWidgetComponent->GetUserWidgetObject());
	if (!SlotWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("SlotWidget is null. SlotIndex: %d"), SlotIndex);
		return;
	}

	if (LinkedPlayerState)
	{
		FString DisplayName = LinkedPlayerState->GetPlayerName();

		EGProjectPlayerLobbyStatus CurrentStatus = LinkedPlayerState->GetPlayerLobbyStatus();
		FString StatusText = UEnum::GetDisplayValueAsText(CurrentStatus).ToString().ToUpper();

		SlotWidget->SetSlotInfo(DisplayName, StatusText);
	}
	else
	{
		SlotWidget->SetSlotInfo(TEXT("[ Empty ]"), TEXT("-"));
	}
}

void AGProjectLobbyPlayerStartSlot::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AGProjectLobbyPlayerStartSlot, LinkedPlayerState);
}

void AGProjectLobbyPlayerStartSlot::OnRep_LinkedPlayerState()
{
	if (LinkedPlayerState)
	{
		LinkedPlayerState->OnLobbyStatusChanged.RemoveAll(this);
		LinkedPlayerState->OnLobbyStatusChanged.AddUObject(this, &AGProjectLobbyPlayerStartSlot::RefreshSlotUI);
	}
	RefreshSlotUI();
}