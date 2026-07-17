// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "GProjectSessionSubsystem.generated.h"

/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGProjectOnCreateSessionComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGProjectOnFindSessionsComplete, const TArray<FString>&, SessionNames, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGProjectOnJoinSessionComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGProjectOnLoginComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGProjectOnDestroySessionComplete, bool, bWasSuccessful);

UCLASS()
class PROJECTG_API UGProjectSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	void CreateGameSession(
		int32 MaxPublicConnections,
		FName SessionNameSetting,
		const FString& RoomName,
		const FString& BattleMapPath
	);
	void FindGameSessions();
	void JoinGameSession(int32 SessionIndex);
	void DestroyGameSession();
	void ExitMatch(APlayerController* RequestingPlayer);

	UFUNCTION(BlueprintCallable, Category = "Online")
	void LoginWithEOS();

	virtual void Deinitialize() override;

	bool GetSessionPlayerCounts(int32 SessionIndex, int32& OutCurrentPlayers, int32& OutMaxPlayers) const;

	UPROPERTY(BlueprintAssignable, Category = "Session")
	FGProjectOnCreateSessionComplete OnCreateSessionCompleteEvent;

	UPROPERTY(BlueprintAssignable, Category = "Session")
	FGProjectOnFindSessionsComplete OnFindSessionsCompleteEvent;

	UPROPERTY(BlueprintAssignable, Category = "Session")
	FGProjectOnJoinSessionComplete OnJoinSessionCompleteEvent;

	UPROPERTY(BlueprintAssignable, Category = "Online")
	FGProjectOnLoginComplete OnLoginCompleteEvent;

	UPROPERTY(BlueprintAssignable, Category = "Session")
	FGProjectOnDestroySessionComplete OnDestroySessionCompleteEvent;

	UFUNCTION(BlueprintCallable, Category = "Loading")
	void ShowLoading();

	UFUNCTION(BlueprintCallable, Category = "Loading")
	void HideLoading();

	UFUNCTION(BlueprintCallable, Category = "Loading")
	bool IsLoadingVisible() const;

protected:
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);

private:
	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	FDelegateHandle CreateSessionCompleteDelegateHandle;
	FDelegateHandle FindSessionsCompleteDelegateHandle;
	FDelegateHandle JoinSessionCompleteDelegateHandle;
	FDelegateHandle LoginCompleteDelegateHandle;
	FDelegateHandle DestroySessionCompleteDelegateHandle;

	FName LobbyMapPath = TEXT("/Game/Level/LobbyMap");

	UPROPERTY()
	TSubclassOf<UUserWidget> LoadingWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> LoadingWidgetInstance;

	void LoadLoadingWidgetClassIfNeeded();
	APlayerController* GetOwningPlayerController() const;

	int32 CachedMaxPlayersForTravel;
	FString CachedBattleMapPathForTravel;
	FText CachedPlayerName;

	TWeakObjectPtr<APlayerController> PendingExitPlayer;
};
