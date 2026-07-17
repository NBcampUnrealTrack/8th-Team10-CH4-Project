// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Player/GProjectPlayerState.h"
#include "GProjectLobbyPlayerStartSlot.generated.h"

UCLASS()
class PROJECTG_API AGProjectLobbyPlayerStartSlot : public AActor
{
	GENERATED_BODY()
	
public:	
	AGProjectLobbyPlayerStartSlot();

	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lobby")
	TObjectPtr<class USceneComponent> SpawnPoint;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lobby")
	int32 SlotIndex = 0;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_LinkedPlayerState)
	TObjectPtr<class AGProjectPlayerState> LinkedPlayerState;

	UFUNCTION()
	void OnRep_LinkedPlayerState();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<class UWidgetComponent> SlotInfoWidgetComponent;

	void LinkPlayer(class AGProjectPlayerState* InPlayerState);

	void UnlinkPlayer();

	void RefreshSlotUI(EGProjectPlayerLobbyStatus NewStatus = EGProjectPlayerLobbyStatus::Wait);

	void OnPlayerNameChanged(const FString& NewName);
};
