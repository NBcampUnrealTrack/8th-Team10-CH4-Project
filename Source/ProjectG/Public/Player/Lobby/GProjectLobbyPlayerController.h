// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GProjectLobbyPlayerController.generated.h"

class UUserWidget;
class UGProjectLobbyWidget;

UCLASS()
class PROJECTG_API AGProjectLobbyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AGProjectLobbyPlayerController();

	virtual void BeginPlay() override;

	UFUNCTION(Client, Reliable)
	void ClientUpdatePlayerCount(int32 CurrentPlayers, int32 RequiredPlayers);

	UFUNCTION(Server, Reliable)
	void Server_ToggleReady();

	UFUNCTION(Server, Reliable)
	void Server_RequestStartGame();

	class UGProjectLobbyWidget* GetLobbyWidget() const;

	void BindLobbyWidgetToPlayerState();

	UFUNCTION(Client, Reliable)
	void ClientRefreshLobbyUI();

protected:
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetPlayerName(const FString& InName);

	virtual void OnRep_PlayerState() override;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> MenuWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> LobbyWidgetClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UUserWidget> CurrentWidgetInstance;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UUserWidget> LobbyWidgetInstance;

	int32 PendingCurrentPlayers = 0;
	int32 PendingRequiredPlayers = 0;
	bool bHasPendingUpdate = false;

private:
	void ShowMenuUI();
	void ShowLobbyUI();
	void ClearCurrentWidget();

	void ApplyPlayerCountToLobbyWidget(int32 CurrentPlayers, int32 RequiredPlayers);
};
