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
UCLASS()
class PROJECTG_API UGProjectSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	UGProjectSessionSubsystem();

	void CreateGameSession(int32 MaxPublicConnections, FName SessionNameSetting);
	void FindGameSessions();
	void JoinGameSession(int32 SessionIndex);

	virtual void Deinitialize() override;

	UPROPERTY(BlueprintAssignable, Category = "Session")
	FGProjectOnCreateSessionComplete OnCreateSessionCompleteEvent;

	UPROPERTY(BlueprintAssignable, Category = "Session")
	FGProjectOnFindSessionsComplete OnFindSessionsCompleteEvent;

	UPROPERTY(BlueprintAssignable, Category = "Session")
	FGProjectOnJoinSessionComplete OnJoinSessionCompleteEvent;

protected:
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

private:
	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	FDelegateHandle CreateSessionCompleteDelegateHandle;
	FDelegateHandle FindSessionsCompleteDelegateHandle;
	FDelegateHandle JoinSessionCompleteDelegateHandle;
};
