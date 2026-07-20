// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/GProjectPlayerController.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/GProjectAbilitySystemComponent.h"
#include "AbilitySystem/GProjectAttributeSet.h"
#include "Character/GProjectCharacter.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Character.h"
#include "GProjectGameplayTags.h"
#include "Input/GProjectInputComponent.h"
#include "Input/GProjectInputConfig.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "Player/GProjectPlayerState.h"
#include "Camera/PlayerCameraManager.h"
#include "UI/HUD/GProjectHUD.h"
#include "Game/GProjectGameState.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "UI/Widget/GProjectChatWidget.h"
#include "Subsystem/GProjectPlayerInfoSubsystem.h"

namespace
{
	constexpr int32 MaxChatMessageLength = 120;
}

AGProjectPlayerController::AGProjectPlayerController()
{
}

void AGProjectPlayerController::SendChatMessage(const FString& Message)
{
	FString SanitizedMessage = Message;
	SanitizedMessage.TrimStartAndEndInline();

	if (SanitizedMessage.IsEmpty())
	{
		return;
	}

	SanitizedMessage = SanitizedMessage.Left(MaxChatMessageLength);
	ServerSendChatMessage(SanitizedMessage);
}

void AGProjectPlayerController::RegisterChatWidget(UGProjectChatWidget* InChatWidget)
{
	ChatWidget = InChatWidget;
}

void AGProjectPlayerController::UnRegisterChatWidget(UGProjectChatWidget* InChatWidget)
{
	if (ChatWidget.Get() != InChatWidget)
	{
		return;
	}

	ChatWidget.Reset();
	bChatOpen = false;
}

void AGProjectPlayerController::OpenChat()
{
	if (bChatOpen || !ChatWidget.IsValid())
	{
		return;
	}

	bChatOpen = true;
	ChatWidget->OpenChatInput();
}

void AGProjectPlayerController::CloseChat()
{
	bChatOpen = false;

	if (ChatWidget.IsValid())
	{
		ChatWidget->CloseChatInput();
	}

	UWidgetBlueprintLibrary::SetInputMode_GameOnly(this, false);
}

void AGProjectPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController())
	{
		if (UGameInstance* GameInstance = GetGameInstance())
		{
			if (UGProjectPlayerInfoSubsystem* PlayerInfoSubsystem =
				GameInstance->GetSubsystem<UGProjectPlayerInfoSubsystem>())
			{
				ServerSetPlayerName(PlayerInfoSubsystem->GetPlayerName());
			}
		}
	}

	bShowMouseCursor = false;

	FInputModeGameOnly InputModeData;
	SetInputMode(InputModeData);

	if (!DefaultMappingContext)
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (!InputSubsystem)
	{
		return;
	}

	InputSubsystem->AddMappingContext(DefaultMappingContext, 0);

	InitHUD();
}

void AGProjectPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	GProjectAbilitySystemComponent = nullptr;
	InitHUD();
}

void AGProjectPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	GProjectAbilitySystemComponent = nullptr;
	InitHUD();
}

void AGProjectPlayerController::SetupInputComponent()
{
	if (!InputComponent)
	{
		InputComponent = NewObject<UGProjectInputComponent>(this, TEXT("PC_GProjectInputComponent0"));
		InputComponent->RegisterComponent();
	}

	Super::SetupInputComponent();

	UGProjectInputComponent* GProjectInputComponent = CastChecked<UGProjectInputComponent>(InputComponent);

	if (MoveAction)
	{
		GProjectInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Move);
	}

	if (JumpAction)
	{
		GProjectInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ThisClass::JumpPressed);
		GProjectInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ThisClass::JumpReleased);
	}

	if (ChatAction)
	{
		GProjectInputComponent->BindAction(ChatAction, ETriggerEvent::Started, this, &ThisClass::OpenChat);
	}

	if (!InputConfig)
	{
		UE_LOG(LogTemp, Warning, TEXT("InputConfig is not set on %s."), *GetNameSafe(this));
		return;
	}
	
	if (SpectatePrevAction)
	{
		GProjectInputComponent->BindAction(SpectatePrevAction, ETriggerEvent::Started, this, &ThisClass::SpectatePrev);
	}
	if (SpectateNextAction)
	{
		GProjectInputComponent->BindAction(SpectateNextAction, ETriggerEvent::Started, this, &ThisClass::SpectateNext);
	}
	
	GProjectInputComponent->BindAbilityActions(InputConfig, this, &ThisClass::AbilityInputTagPressed, &ThisClass::AbilityInputTagReleased, &ThisClass::AbilityInputTagHeld);
}

UGProjectAbilitySystemComponent* AGProjectPlayerController::GetASC()
{
	if (!GProjectAbilitySystemComponent)
	{
		GProjectAbilitySystemComponent = Cast<UGProjectAbilitySystemComponent>(
			UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn())
		);
	}

	return GProjectAbilitySystemComponent;
}

bool AGProjectPlayerController::IsGameplayInputBlocked()
{
	const AGProjectGameState* GS = GetWorld() ? GetWorld()->GetGameState<AGProjectGameState>() : nullptr;

	if (GS)
	{
		const bool bMatchNotPlaying = !GS->IsMatchInProgress();

		const bool bRoundNotPlaying = GS->GetRoundPhase() != ERoundPhase::Playing;

		if (bMatchNotPlaying || bRoundNotPlaying)
		{
			return true;
		}
	}

	if (bChatOpen)
	{
		return true;
	}

	if (UGProjectAbilitySystemComponent* ASC = GetASC())
	{
		return ASC->HasMatchingGameplayTag(GProjectGameplayTags::State_Character_Dead) ||
			ASC->HasMatchingGameplayTag(GProjectGameplayTags::State_Combat_Hitstun) ||
			ASC->HasMatchingGameplayTag(GProjectGameplayTags::State_Combat_Knockdown) ||
			ASC->HasMatchingGameplayTag(GProjectGameplayTags::State_Combat_Parrying) ||
			ASC->HasMatchingGameplayTag(GProjectGameplayTags::State_Interaction_Pickup);
	}

	return false;
}

void AGProjectPlayerController::InitHUD()
{
	if (!IsLocalController())
	{
		return;
	}

	AGProjectPlayerState* GProjectPlayerState = GetPlayerState<AGProjectPlayerState>();
	if (!GProjectPlayerState)
	{
		return;
	}

	AGProjectHUD* GProjectHUD = Cast<AGProjectHUD>(GetHUD());
	if (!GProjectHUD)
	{
		return;
	}

	GProjectHUD->InitOverlay(this, GProjectPlayerState, GProjectPlayerState->GetGProjectAbilitySystemComponent(), GProjectPlayerState->GetAttributeSet());
}

void AGProjectPlayerController::Move(const FInputActionValue& InputActionValue)
{
	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();
	if (InputAxisVector.IsNearlyZero())
	{
		return;
	}

	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
	{
		return;
	}

	if (IsGameplayInputBlocked())
	{
		return;
	}

	// 공격 몽타주(콤보) 재생 중에는 이동 입력 차단
	if (const UGProjectAbilitySystemComponent* ASC = GetASC())
	{
		if (ASC->HasMatchingGameplayTag(GProjectGameplayTags::State_Combat_Attacking))
		{
			return;
		}
	}

	const FRotator YawRotation(0.0f, GetControlRotation().Yaw, 0.0f);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	ControlledPawn->AddMovementInput(ForwardDirection, InputAxisVector.Y);
	ControlledPawn->AddMovementInput(RightDirection, InputAxisVector.X);
}

void AGProjectPlayerController::JumpPressed()
{
	if (IsGameplayInputBlocked())
	{
		return;
	}

	if (ACharacter* ControlledCharacter = GetCharacter())
	{
		ControlledCharacter->Jump();
	}
}

void AGProjectPlayerController::JumpReleased()
{
	if (ACharacter* ControlledCharacter = GetCharacter())
	{
		ControlledCharacter->StopJumping();
	}
}

void AGProjectPlayerController::AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (IsGameplayInputBlocked())
	{
		return;
	}

	if (InputTag.MatchesTagExact(GProjectGameplayTags::InputTag_Combat_BasicAttack) ||
		InputTag.MatchesTagExact(GProjectGameplayTags::InputTag_Combat_StrongAttack))
	{
		HandleAttackInputPressed(InputTag);
		return;
	}

	if (UGProjectAbilitySystemComponent* ASC = GetASC())
	{
		ASC->AbilityInputTagPressed(InputTag);
	}
}

void AGProjectPlayerController::AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (InputTag.MatchesTagExact(GProjectGameplayTags::InputTag_Combat_BasicAttack))
	{
		bBasicAttackHeld = false;
		return;
	}

	if (InputTag.MatchesTagExact(GProjectGameplayTags::InputTag_Combat_StrongAttack))
	{
		bStrongAttackHeld = false;
		return;
	}

	if (UGProjectAbilitySystemComponent* ASC = GetASC())
	{
		ASC->AbilityInputTagReleased(InputTag);
	}
}

void AGProjectPlayerController::AbilityInputTagHeld(FGameplayTag InputTag)
{
	if (IsGameplayInputBlocked())
	{
		return;
	}

	if (UGProjectAbilitySystemComponent* ASC = GetASC())
	{
		ASC->AbilityInputTagHeld(InputTag);
	}
}

void AGProjectPlayerController::HandleAttackInputPressed(FGameplayTag InputTag)
{
	if (InputTag.MatchesTagExact(GProjectGameplayTags::InputTag_Combat_BasicAttack))
	{
		bBasicAttackHeld = true;
	}
	else if (InputTag.MatchesTagExact(GProjectGameplayTags::InputTag_Combat_StrongAttack))
	{
		bStrongAttackHeld = true;
	}
	else
	{
		return;
	}

	const bool bOppositePending =
		(PendingAttackInputTag.MatchesTagExact(GProjectGameplayTags::InputTag_Combat_BasicAttack) &&
			InputTag.MatchesTagExact(GProjectGameplayTags::InputTag_Combat_StrongAttack)) ||
		(PendingAttackInputTag.MatchesTagExact(GProjectGameplayTags::InputTag_Combat_StrongAttack) &&
			InputTag.MatchesTagExact(GProjectGameplayTags::InputTag_Combat_BasicAttack));

	if (bOppositePending && bBasicAttackHeld && bStrongAttackHeld)
	{
		TryStartParryInput();
		return;
	}

	if (PendingAttackInputTag.IsValid())
	{
		return;
	}

	PendingAttackInputTag = InputTag;
	GetWorldTimerManager().SetTimer(
		PendingAttackInputTimer,
		this,
		&ThisClass::FlushPendingAttackInput,
		ParryChordWindow,
		false);
}

void AGProjectPlayerController::FlushPendingAttackInput()
{
	if (PendingAttackInputTag.IsValid())
	{
		const FGameplayTag AttackInputTag = PendingAttackInputTag;
		ClearPendingAttackInput();
		SendAttackInputEvent(AttackInputTag);
	}
}

void AGProjectPlayerController::ClearPendingAttackInput()
{
	GetWorldTimerManager().ClearTimer(PendingAttackInputTimer);
	PendingAttackInputTag = FGameplayTag();
}

void AGProjectPlayerController::TryStartParryInput()
{
	ClearPendingAttackInput();

	if (UGProjectAbilitySystemComponent* ASC = GetASC())
	{
		ASC->AbilityInputTagPressed(GProjectGameplayTags::InputTag_Combat_Parry);
	}
}

void AGProjectPlayerController::SendAttackInputEvent(FGameplayTag InputTag)
{
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
	{
		return;
	}

	FGameplayTag EventTag;
	if (InputTag.MatchesTagExact(GProjectGameplayTags::InputTag_Combat_BasicAttack))
	{
		EventTag = GProjectGameplayTags::Event_Input_Combat_BasicAttack;
	}
	else if (InputTag.MatchesTagExact(GProjectGameplayTags::InputTag_Combat_StrongAttack))
	{
		EventTag = GProjectGameplayTags::Event_Input_Combat_StrongAttack;
	}
	else
	{
		return;
	}

	FGameplayEventData EventData;
	EventData.EventTag = EventTag;
	EventData.Instigator = ControlledPawn;
	EventData.Target = ControlledPawn;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(ControlledPawn, EventTag, EventData);

	if (!HasAuthority())
	{
		ServerSendAttackInputEvent(InputTag);
	}
}

void AGProjectPlayerController::ServerSendChatMessage_Implementation(const FString& Message)
{
	FString SanitizedMessage = Message;
	SanitizedMessage.TrimStartAndEndInline();
	SanitizedMessage = SanitizedMessage.Left(MaxChatMessageLength);

	if (SanitizedMessage.IsEmpty())
	{
		return;
	}

	const AGProjectPlayerState* PS = GetPlayerState<AGProjectPlayerState>();

	if (!PS) 
	{
		return;
	}
	
	const FString SenderName = PS->GetPlayerName();
	const int32 SenderPlayerID = PS->GetPlayerId();

	AGProjectGameState* GS = GetWorld()->GetGameState<AGProjectGameState>();
	if (!GS)
	{
		return;
	}

	GS->BroadcastChatMessage(SenderPlayerID, SenderName, SanitizedMessage);
	
}

void AGProjectPlayerController::ServerSendAttackInputEvent_Implementation(FGameplayTag InputTag)
{
	SendAttackInputEvent(InputTag);
}

void AGProjectPlayerController::SpectatePrev()
{
	APawn* MyPawn = GetPawn();
	bool bIsDead = true;
	if (AGProjectCharacter* MyChar = Cast<AGProjectCharacter>(MyPawn))
	{
		bIsDead = MyChar->IsDead();
	}

	if (bIsDead)
	{
		ServerChangeSpectateTarget(-1); 
	}
}

void AGProjectPlayerController::SpectateNext()
{
	bool bIsDead = true;
	if (AGProjectCharacter* MyChar = Cast<AGProjectCharacter>(GetPawn()))
	{
		bIsDead = MyChar->IsDead();
	}

	if (bIsDead)
	{
		ServerChangeSpectateTarget(1); 
	}
}

void AGProjectPlayerController::ServerChangeSpectateTarget_Implementation(int32 Direction)
{
	TArray<AGProjectCharacter*> LivingCharacters;
    
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC) continue;

		AGProjectCharacter* LoopChar = Cast<AGProjectCharacter>(PC->GetPawn());
		if (LoopChar && !LoopChar->IsDead())
		{
			LivingCharacters.Add(LoopChar);
		}
	}
	
	if (LivingCharacters.Num() == 0) return;
	
	CurrentSpectateIndex += Direction;
	if (CurrentSpectateIndex >= LivingCharacters.Num())
	{
		CurrentSpectateIndex = 0;
	}
	else if (CurrentSpectateIndex < 0)
	{
		CurrentSpectateIndex = LivingCharacters.Num() - 1;
	}
	
	if (LivingCharacters.IsValidIndex(CurrentSpectateIndex))
	{
		SetViewTargetWithBlend(LivingCharacters[CurrentSpectateIndex], 0.3f);
	}
}


void AGProjectPlayerController::ServerRequestReturnToLobby_Implementation()
{
	if (UWorld* World = GetWorld())
	{
		World->ServerTravel(TEXT("/Game/Level/LobbyMap?listen"));
	}
}

void AGProjectPlayerController::ServerSetPlayerName_Implementation(const FString& InName)
{
	FString SanitizedName = InName;
	SanitizedName.TrimStartAndEndInline();
	SanitizedName = SanitizedName.Left(24);

	if (SanitizedName.IsEmpty())
	{
		return;
	}

	if (AGProjectPlayerState* PS = GetPlayerState<AGProjectPlayerState>())
	{
		PS->SetPlayerName(SanitizedName);
	}
}
