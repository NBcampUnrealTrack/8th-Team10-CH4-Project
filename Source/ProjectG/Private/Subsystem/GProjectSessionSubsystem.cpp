
#include "Subsystem/GProjectSessionSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"

UGProjectSessionSubsystem::UGProjectSessionSubsystem()
{

}

void UGProjectSessionSubsystem::CreateGameSession(int32 MaxPublicConnections, FName SessionNameSetting)
{
	IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	if (!Subsystem)
	{
		OnCreateSessionCompleteEvent.Broadcast(false);
		return;
	}

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
		OnCreateSessionCompleteEvent.Broadcast(false);
		return;
	}

	CreateSessionCompleteDelegateHandle =
		SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(
			FOnCreateSessionCompleteDelegate::CreateUObject(
				this,
				&UGProjectSessionSubsystem::OnCreateSessionComplete
			)
		);

			ShowLoading();

	FOnlineSessionSettings SessionSettings;
	SessionSettings.bIsLANMatch = Subsystem->GetSubsystemName() == "NULL";
	SessionSettings.NumPublicConnections = MaxPublicConnections;
	SessionSettings.bAllowInvites = true;
	SessionSettings.bAllowJoinInProgress = true;
	SessionSettings.bShouldAdvertise = true;
	SessionSettings.bUsesPresence = true;
	SessionSettings.bAllowJoinViaPresence = true;

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

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!LocalPlayer ||
		!SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SessionSettings))
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
		HideLoading();
		OnCreateSessionCompleteEvent.Broadcast(false);
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

		UGameplayStatics::OpenLevel(
			GetWorld(),
			LobbyMapPath,
			true,
			"listen"
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
	if (!Subsystem)
	{
		return;
	}

	SessionInterface = Subsystem->GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		return;
	}

	if (!SessionInterface.IsValid()) return;

	FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(
		FOnFindSessionsCompleteDelegate::CreateUObject(this, &UGProjectSessionSubsystem::OnFindSessionsComplete)
	);

	ShowLoading();

	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	SessionSearch->MaxSearchResults = 10000;
	SessionSearch->bIsLanQuery = (IOnlineSubsystem::Get()->GetSubsystemName() == "NULL");

	SessionSearch->QuerySettings.Set(FName(TEXT("PRESENCE")), true, EOnlineComparisonOp::Equals);

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), SessionSearch.ToSharedRef()))
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
		HideLoading();
		OnFindSessionsCompleteEvent.Broadcast(TArray<FString>(), false);
	}
}

void UGProjectSessionSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
	}

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
			FString MapName;
			SessionResult.Session.SessionSettings.Get(FName(TEXT("MAPNAME")), MapName);

			FString OwnerName = SessionResult.Session.OwningUserName;
			DisplayNames.Add(FString::Printf(TEXT("%s's Room (%s)"), *OwnerName, *MapName));
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
	if (!SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SessionSearch->SearchResults[SessionIndex]))
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
		HideLoading();
		OnJoinSessionCompleteEvent.Broadcast(false);
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