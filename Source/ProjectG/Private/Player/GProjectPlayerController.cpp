// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/GProjectPlayerController.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/GProjectAbilitySystemComponent.h"
#include "AbilitySystem/GProjectAttributeSet.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Character.h"
#include "GProjectGameplayTags.h"
#include "Input/GProjectInputComponent.h"
#include "Input/GProjectInputConfig.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "Player/GProjectPlayerState.h"
#include "UI/HUD/GProjectHUD.h"

AGProjectPlayerController::AGProjectPlayerController()
{
}

void AGProjectPlayerController::BeginPlay()
{
	Super::BeginPlay();

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

	if (!InputConfig)
	{
		UE_LOG(LogTemp, Warning, TEXT("InputConfig is not set on %s."), *GetNameSafe(this));
		return;
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

	if (UGProjectAbilitySystemComponent* ASC = GetASC(); ASC && ASC->HasMatchingGameplayTag(GProjectGameplayTags::State_Dead))
	{
		return;
	}

	const FRotator YawRotation(0.0f, GetControlRotation().Yaw, 0.0f);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	ControlledPawn->AddMovementInput(ForwardDirection, InputAxisVector.Y);
	ControlledPawn->AddMovementInput(RightDirection, InputAxisVector.X);
}

void AGProjectPlayerController::JumpPressed()
{
	if (UGProjectAbilitySystemComponent* ASC = GetASC(); ASC && ASC->HasMatchingGameplayTag(GProjectGameplayTags::State_Dead))
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
	if (UGProjectAbilitySystemComponent* ASC = GetASC())
	{
		ASC->AbilityInputTagPressed(InputTag);
	}
}

void AGProjectPlayerController::AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (UGProjectAbilitySystemComponent* ASC = GetASC())
	{
		ASC->AbilityInputTagReleased(InputTag);
	}
}

void AGProjectPlayerController::AbilityInputTagHeld(FGameplayTag InputTag)
{
	if (UGProjectAbilitySystemComponent* ASC = GetASC())
	{
		ASC->AbilityInputTagHeld(InputTag);
	}
}
