
#include "Subsystem/GProjectSessionSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSessionSettings.h"
#include "Online/OnlineSessionNames.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"

void UGProjectSessionSubsystem::LoginWithEOS()
{
	IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	if (!Subsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("LoginWithEOS failed: Subsystem is null"));
		OnLoginCompleteEvent.Broadcast(false);
		return;
	}

	if (Subsystem->GetSubsystemName() == TEXT("NULL"))
	{
		UE_LOG(LogTemp, Warning, TEXT("NULL subsystem: skip login"));
		OnLoginCompleteEvent.Broadcast(true);
		return;
	}

	IOnlineIdentityPtr IdentityInterface = Subsystem->GetIdentityInterface();
	if (!IdentityInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("LoginWithEOS failed: IdentityInterface is invalid"));
		OnLoginCompleteEvent.Broadcast(false);
		return;
	}

	if (IdentityInterface->GetLoginStatus(0) == ELoginStatus::LoggedIn)
	{
		UE_LOG(LogTemp, Log, TEXT("Already Logged In to EOS"));
		OnLoginCompleteEvent.Broadcast(true);
		return;
	}

	LoginCompleteDelegateHandle = IdentityInterface->AddOnLoginCompleteDelegate_Handle(
		0,
		FOnLoginCompleteDelegate::CreateUObject(this, &UGProjectSessionSubsystem::OnLoginComplete)
	);

	ShowLoading();

	FOnlineAccountCredentials Credentials;

	//AccountPortal / Developer /  ExchangeCode
	Credentials.Type = TEXT("accountportal");
	Credentials.Id = TEXT("");
	Credentials.Token = TEXT("");

	/*FString AuthType;
	FString AuthLogin;
	FString AuthPassword;

	FParse::Value(FCommandLine::Get(), TEXT("auth_type="), AuthType);
	FParse::Value(FCommandLine::Get(), TEXT("auth_login="), AuthLogin);
	FParse::Value(FCommandLine::Get(), TEXT("auth_password="), AuthPassword);

	Credentials.Type = AuthType.IsEmpty() ? TEXT("developer") : AuthType;
	Credentials.Id = AuthLogin;
	Credentials.Token = AuthPassword;*/

	if (!IdentityInterface->Login(0, Credentials))
	{
		UE_LOG(LogTemp, Error, TEXT("IdentityInterface->Login call failed"));
		IdentityInterface->ClearOnLoginCompleteDelegate_Handle(0, LoginCompleteDelegateHandle);
		HideLoading();
		OnLoginCompleteEvent.Broadcast(false);
	}
}

void UGProjectSessionSubsystem::OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error)
{
	IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	if (Subsystem)
	{
		IOnlineIdentityPtr IdentityInterface = Subsystem->GetIdentityInterface();
		if (IdentityInterface.IsValid())
		{
			IdentityInterface->ClearOnLoginCompleteDelegate_Handle(LocalUserNum, LoginCompleteDelegateHandle);
		}
	}



	UE_LOG(LogTemp, Warning, TEXT("Subsystem: %s"), *Subsystem->GetSubsystemName().ToString());
	UE_LOG(LogTemp, Warning, TEXT("UserId Valid: %s"), UserId.IsValid() ? TEXT("true") : TEXT("false"));

	if (UserId.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("UserId: %s"), *UserId.ToString());
	}

	HideLoading();
	UE_LOG(LogTemp, Log, TEXT("EOS Login Complete. Success: %d, Error: %s"), bWasSuccessful, *Error);

	OnLoginCompleteEvent.Broadcast(bWasSuccessful);
}

void UGProjectSessionSubsystem::CreateGameSession(
	int32 MaxPublicConnections,
	FName SessionNameSetting,
	const FString& RoomName,
	const FString& BattleMapPath
) {
	IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	if (!Subsystem)
	{
		OnCreateSessionCompleteEvent.Broadcast(false);
		return;
	}

	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::White, FString::Printf(TEXT("OSS is %s"), *Subsystem->GetSubsystemName().ToString()));

	SessionInterface = Subsystem->GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		OnCreateSessionCompleteEvent.Broadcast(false);
		return;
	}

	FNamedOnlineSession* ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
	if (ExistingSession != nullptr)
	{
		SessionInterface->DestroySession(NAME_GameSession);
	}

	CreateSessionCompleteDelegateHandle =
		SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(
			FOnCreateSessionCompleteDelegate::CreateUObject(this, &UGProjectSessionSubsystem::OnCreateSessionComplete)
		);


	ShowLoading();

	CachedMaxPlayersForTravel = MaxPublicConnections;
	CachedBattleMapPathForTravel = BattleMapPath;

	FOnlineSessionSettings SessionSettings;
	SessionSettings.bIsLANMatch = (Subsystem->GetSubsystemName() == "NULL");
	SessionSettings.NumPublicConnections = MaxPublicConnections;
	SessionSettings.bShouldAdvertise = true;
	SessionSettings.bUsesPresence = true;
	SessionSettings.bUseLobbiesIfAvailable = true;

	SessionSettings.Set(
		FName(TEXT("MaxPlayersCustom")),
		MaxPublicConnections,
		EOnlineDataAdvertisementType::ViaOnlineServiceAndPing
	);

	SessionSettings.Set(
		FName(TEXT("MAPNAME")),
		SessionNameSetting.ToString(),
		EOnlineDataAdvertisementType::ViaOnlineServiceAndPing
	);

	SessionSettings.Set(
		FName(TEXT("ROOM_NAME")),
		RoomName,
		EOnlineDataAdvertisementType::ViaOnlineServiceAndPing
	);

	SessionSettings.Set(
		FName(TEXT("BATTLE_MAP_PATH")),
		BattleMapPath,
		EOnlineDataAdvertisementType::ViaOnlineServiceAndPing
	);

	SessionSettings.Set(
		FName("MATCH_TYPE"),
		FString("Default"),
		EOnlineDataAdvertisementType::ViaOnlineService
	);

	FString MatchType;
	SessionSettings.Get(FName(TEXT("MATCH_TYPE")), MatchType);

	UE_LOG(LogTemp, Warning, TEXT("Create MATCH_TYPE: %s"), *MatchType);
	UE_LOG(LogTemp, Warning, TEXT("Create Advertise: %d"), SessionSettings.bShouldAdvertise);
	UE_LOG(LogTemp, Warning, TEXT("Create UseLobbies: %d"), SessionSettings.bUseLobbiesIfAvailable);
	UE_LOG(LogTemp, Warning, TEXT("Create Presence: %d"), SessionSettings.bUsesPresence);
	UE_LOG(LogTemp, Warning, TEXT("Create LAN: %d"), SessionSettings.bIsLANMatch);
	UE_LOG(LogTemp, Warning, TEXT("Create MaxPlayers: %d"), SessionSettings.NumPublicConnections);

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!LocalPlayer)
	{
		UE_LOG(LogTemp, Error, TEXT("CreateSession failed: LocalPlayer is null"));
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
		HideLoading();
		OnCreateSessionCompleteEvent.Broadcast(false);
		return;
	}

	FUniqueNetIdRepl UserId = LocalPlayer->GetPreferredUniqueNetId();
	if (!UserId.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("CreateSession failed: UserId invalid. Are you logged in?"));
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
		HideLoading();
		OnCreateSessionCompleteEvent.Broadcast(false);
		return;
	}

	if (!SessionInterface->CreateSession(*UserId, NAME_GameSession, SessionSettings))
	{
		UE_LOG(LogTemp, Error, TEXT("CreateSession call failed"));
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
		HideLoading();
		OnCreateSessionCompleteEvent.Broadcast(false);
		return;
	}
}

void UGProjectSessionSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	}

	OnCreateSessionCompleteEvent.Broadcast(bWasSuccessful);

	if (bWasSuccessful)
	{
		ShowLoading();

		FString OptionsString = FString::Printf(TEXT("listen?MaxPlayersCustom=%d?BATTLE_MAP_PATH=%s"),
			CachedMaxPlayersForTravel, *CachedBattleMapPathForTravel);

		UGameplayStatics::OpenLevel(
			GetWorld(),
			LobbyMapPath,
			true,
			OptionsString
		);
	}
	else
	{
		HideLoading();
	}
}

void UGProjectSessionSubsystem::FindGameSessions()
{
	IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	if (!Subsystem) return;

	SessionInterface = Subsystem->GetSessionInterface();
	if (!SessionInterface.IsValid()) return;

	FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(
		FOnFindSessionsCompleteDelegate::CreateUObject(this, &UGProjectSessionSubsystem::OnFindSessionsComplete)
	);

	ShowLoading();

	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	SessionSearch->MaxSearchResults = 100;
	SessionSearch->bIsLanQuery = (Subsystem->GetSubsystemName() == "NULL");

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	FUniqueNetIdRepl UserId = LocalPlayer->GetPreferredUniqueNetId();

	SessionSearch->QuerySettings.Set(
		SEARCH_LOBBIES,
		true,
		EOnlineComparisonOp::Equals
	);

	SessionSearch->QuerySettings.Set(
		FName(TEXT("MATCH_TYPE")),
		FString(TEXT("Default")),
		EOnlineComparisonOp::Equals
	);

	UE_LOG(LogTemp, Warning, TEXT("Find OSS: %s"), *Subsystem->GetSubsystemName().ToString());
	UE_LOG(LogTemp, Warning, TEXT("Find LAN: %d"), SessionSearch->bIsLanQuery);
	UE_LOG(LogTemp, Warning, TEXT("Find MaxResults: %d"), SessionSearch->MaxSearchResults);
	UE_LOG(LogTemp, Warning, TEXT("Find SEARCH_LOBBIES: true"));
	UE_LOG(LogTemp, Warning, TEXT("Find SEARCH_PRESENCE: true"));
	UE_LOG(LogTemp, Warning, TEXT("Find MATCH_TYPE: Default"));

	UE_LOG(LogTemp, Warning, TEXT("SearchParams Num: %d"),
		SessionSearch->QuerySettings.SearchParams.Num());

	bool bStarted = SessionInterface->FindSessions(*UserId, SessionSearch.ToSharedRef());

	UE_LOG(LogTemp, Warning, TEXT("SessionInterface->Fin Started: %d"), bStarted);

	if (!bStarted)
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
		HideLoading();
		OnFindSessionsCompleteEvent.Broadcast(TArray<FString>(), false);
		return;
	}
}

void UGProjectSessionSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
	UE_LOG(LogTemp, Warning, TEXT("FindSessions Success: %d"), bWasSuccessful);
	UE_LOG(LogTemp, Warning, TEXT("SearchResults Num: %d"), SessionSearch.IsValid() ? SessionSearch->SearchResults.Num() : -1);

	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
	}

	TArray<FString> DisplayNames;
	if (bWasSuccessful && SessionSearch.IsValid())
	{
		for (int32 i = 0; i < SessionSearch->SearchResults.Num(); ++i)
		{
			auto& SessionResult = SessionSearch->SearchResults[i];

			FString CustomRoomName;
			SessionResult.Session.SessionSettings.Get(FName(TEXT("ROOM_NAME")), CustomRoomName);

			FString OwnerName = SessionResult.Session.OwningUserName;

			// Room Name
			FString FinalRoomName = CustomRoomName.IsEmpty() ? FString::Printf(TEXT("%s's Room"), *OwnerName) : CustomRoomName;

			DisplayNames.Add(FinalRoomName);
		}
	}
	HideLoading();

	OnFindSessionsCompleteEvent.Broadcast(DisplayNames, bWasSuccessful);
}

void UGProjectSessionSubsystem::JoinGameSession(int32 SessionIndex)
{
	IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	if (!Subsystem)
	{
		OnJoinSessionCompleteEvent.Broadcast(false);
		return;
	}

	SessionInterface = Subsystem->GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		OnJoinSessionCompleteEvent.Broadcast(false);
		return;
	}

	if (!SessionInterface.IsValid() || !SessionSearch.IsValid() || SessionSearch->SearchResults.Num() <= SessionIndex)
	{
		OnJoinSessionCompleteEvent.Broadcast(false);
		return;
	}

	JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
		FOnJoinSessionCompleteDelegate::CreateUObject(this, &UGProjectSessionSubsystem::OnJoinSessionComplete)
	);
	ShowLoading();

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!LocalPlayer)
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
		HideLoading();
		OnJoinSessionCompleteEvent.Broadcast(false);
		return;
	}

	FUniqueNetIdRepl UserId = LocalPlayer->GetPreferredUniqueNetId();

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("Join UserId Valid: %s"),
		UserId.IsValid() ? TEXT("true") : TEXT("false")
	);

	if (!UserId.IsValid())
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
		HideLoading();
		OnJoinSessionCompleteEvent.Broadcast(false);
		return;
	}

	if (!SessionInterface->JoinSession(
		*UserId,
		NAME_GameSession,
		SessionSearch->SearchResults[SessionIndex]
	))
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
		HideLoading();
		OnJoinSessionCompleteEvent.Broadcast(false);
		return;
	}

}

void UGProjectSessionSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
	}

	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		FString ConnectInfo;
		if (SessionInterface->GetResolvedConnectString(NAME_GameSession, ConnectInfo))
		{
			APlayerController* PC = GetWorld()->GetFirstPlayerController();
			if (PC)
			{
				ShowLoading();
				PC->ClientTravel(ConnectInfo, ETravelType::TRAVEL_Absolute);
				OnJoinSessionCompleteEvent.Broadcast(true);
				return;
			}
		}
	}
	HideLoading();
	OnJoinSessionCompleteEvent.Broadcast(false);
}

void UGProjectSessionSubsystem::Deinitialize()
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
	}

	SessionSearch.Reset();
	SessionInterface.Reset();

	Super::Deinitialize();
}

bool UGProjectSessionSubsystem::GetSessionPlayerCounts(
	int32 SessionIndex,
	int32& OutCurrentPlayers,
	int32& OutMaxPlayers
) const
{
	if (!SessionSearch.IsValid() || !SessionSearch->SearchResults.IsValidIndex(SessionIndex))
	{
		return false;
	}

	const FOnlineSessionSearchResult& Result = SessionSearch->SearchResults[SessionIndex];

	int32 MaxPlayers = Result.Session.SessionSettings.NumPublicConnections;

	int32 FoundMaxPlayers = 0;
	if (Result.Session.SessionSettings.Get(FName(TEXT("MaxPlayersCustom")), FoundMaxPlayers))
	{
		MaxPlayers = FoundMaxPlayers;
	}

	const int32 OpenConnections = Result.Session.NumOpenPublicConnections;
	const int32 CurrentPlayers = MaxPlayers - OpenConnections;

	OutCurrentPlayers = CurrentPlayers;
	OutMaxPlayers = MaxPlayers;

	return true;
}

void UGProjectSessionSubsystem::ShowLoading()
{
	if (IsLoadingVisible()) return;

	LoadLoadingWidgetClassIfNeeded();
	if (!LoadingWidgetClass) return;

	APlayerController* PC = GetOwningPlayerController();
	if (!PC) return;

	LoadingWidgetInstance = CreateWidget<UUserWidget>(PC, LoadingWidgetClass);
	if (LoadingWidgetInstance)
	{
		LoadingWidgetInstance->AddToViewport(5);

		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(LoadingWidgetInstance->GetCachedWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(true);
	}
}

void UGProjectSessionSubsystem::HideLoading()
{
	if (!LoadingWidgetInstance) return;

	LoadingWidgetInstance->RemoveFromParent();
	LoadingWidgetInstance = nullptr;

	if (APlayerController* PC = GetOwningPlayerController())
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(true);
	}
}

bool UGProjectSessionSubsystem::IsLoadingVisible() const
{
	return LoadingWidgetInstance && LoadingWidgetInstance->IsInViewport();
}

void UGProjectSessionSubsystem::LoadLoadingWidgetClassIfNeeded()
{
	if (!LoadingWidgetClass)
	{
		FSoftClassPath LoadingWidgetClassPath(TEXT("/Game/Main/BP/Widget/UserWidget/Lobby/WBP_LoadingScreenWidget.WBP_LoadingScreenWidget_C"));
		LoadingWidgetClass = LoadingWidgetClassPath.TryLoadClass<UUserWidget>();
	}
}

APlayerController* UGProjectSessionSubsystem::GetOwningPlayerController() const
{
	UWorld* World = GetWorld();
	return World ? World->GetFirstPlayerController() : nullptr;
}
