// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/Lobby/GProjectMenuWidget.h"
#include "Subsystem/GProjectSessionSubsystem.h"
#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Player/Lobby/GProjectLobbyPlayerController.h"
#include "Kismet/GameplayStatics.h"

UGProjectMenuWidget::UGProjectMenuWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UGProjectMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	JoinButton.Get()->OnClicked.AddDynamic(this, &ThisClass::OnJoinButtonClicked);
	ExitButton.Get()->OnClicked.AddDynamic(this, &ThisClass::OnExitButtonClicked);
    HostButton.Get()->OnClicked.AddDynamic(this, &ThisClass::OnHostButtonClicked);
}

void UGProjectMenuWidget::OnHostButtonClicked()
{
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        UGProjectSessionSubsystem* Subsystem = GameInstance->GetSubsystem<UGProjectSessionSubsystem>();
        if (Subsystem)
        {
            // 4인 제한, 'MyAwesomeRoom' 이름으로 방 생성 시작
            Subsystem->CreateGameSession(4, FName("MyAwesomeRoom"));
        }
    }
}

void UGProjectMenuWidget::OnJoinButtonClicked()
{
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        UGProjectSessionSubsystem* Subsystem = GameInstance->GetSubsystem<UGProjectSessionSubsystem>();
        if (Subsystem)
        {
            Subsystem->OnFindSessionsCompleteEvent.RemoveDynamic(this, &ThisClass::OnFindSessionsCompleteUpdateUI);
            Subsystem->OnFindSessionsCompleteEvent.AddDynamic(this, &ThisClass::OnFindSessionsCompleteUpdateUI);
            Subsystem->FindGameSessions();
        }
    }
}

void UGProjectMenuWidget::OnExitButtonClicked()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}

void UGProjectMenuWidget::OnFindSessionsCompleteUpdateUI(const TArray<FString>& SessionNames, bool bWasSuccessful)
{
    if (bWasSuccessful && SessionNames.Num() > 0)
    {
        if (UGameInstance* GameInstance = GetGameInstance())
        {
            UGProjectSessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UGProjectSessionSubsystem>();
            if (SessionSubsystem)
            {
                SessionSubsystem->JoinGameSession(0);
            }
        }
    }
}