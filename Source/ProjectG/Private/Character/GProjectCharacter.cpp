// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/GProjectCharacter.h"

#include "AbilitySystem/GProjectAbilitySystemComponent.h"
#include "AbilitySystem/Abilities/GProjectGameplayAbility.h"
#include "AbilitySystem/Combo/GProjectComboData.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/MeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GProjectGameplayTags.h"
#include "Player/GProjectPlayerState.h"
#include "Targeting/GProjectLockOnComponent.h"
#include "Net/UnrealNetwork.h"

AGProjectCharacter::AGProjectCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	GetCharacterMovement()->SetIsReplicated(true);
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
	
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->AirControl = 0.95f; //공중에서 회전 궤적을 변경하기 어렵게 만들려면 수치 Down
		GetCharacterMovement()->FallingLateralFriction = 0.5f; // 공중에서 관성력 어느정도 줄건가
		GetCharacterMovement()->AirControlBoostMultiplier = 1.0f;
	}

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 1150.0f;
	CameraBoom->SetRelativeRotation(FRotator(-58.0f, 0.0f, 0.0f));
	CameraBoom->bUsePawnControlRotation = false;
	CameraBoom->bInheritPitch = false;
	CameraBoom->bInheritYaw = false;
	CameraBoom->bInheritRoll = false;
	CameraBoom->bDoCollisionTest = false;

	TopDownCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCamera->bUsePawnControlRotation = false;
	TopDownCamera->FieldOfView = 60.0f;

	LockOnComponent = CreateDefaultSubobject<UGProjectLockOnComponent>(TEXT("LockOnComponent"));
}

UAbilitySystemComponent* AGProjectCharacter::GetAbilitySystemComponent() const
{
	return GetGProjectAbilitySystemComponent();
}

UGProjectAbilitySystemComponent* AGProjectCharacter::GetGProjectAbilitySystemComponent() const
{
	const AGProjectPlayerState* GProjectPlayerState = GetPlayerState<AGProjectPlayerState>();
	return GProjectPlayerState ? GProjectPlayerState->GetGProjectAbilitySystemComponent() : nullptr;
}

UGProjectLockOnComponent* AGProjectCharacter::GetLockOnComponent() const
{
	return FindComponentByClass<UGProjectLockOnComponent>();
}

UGProjectComboData* AGProjectCharacter::GetActiveGroundComboData() const
{
	return ActiveGroundComboData ? ActiveGroundComboData.Get() : DefaultGroundComboData.Get();
}

UGProjectComboData* AGProjectCharacter::GetActiveAirComboData() const
{
	return ActiveAirComboData ? ActiveAirComboData.Get() : DefaultAirComboData.Get();
}

UGProjectComboData* AGProjectCharacter::GetActiveDashComboData() const
{
	return ActiveDashComboData ? ActiveDashComboData.Get() : DefaultDashComboData.Get();
}

UMeshComponent* AGProjectCharacter::GetAttackTraceMesh() const
{
	return AttackTraceMesh;
}

FName AGProjectCharacter::GetAttackTraceStartSocketName() const
{
	return AttackTraceStartSocket;
}

FName AGProjectCharacter::GetAttackTraceEndSocketName() const
{
	return AttackTraceEndSocket;
}

void AGProjectCharacter::SetAttackTraceSource(UMeshComponent* InTraceMesh, FName InStartSocket, FName InEndSocket)
{
	AttackTraceMesh = InTraceMesh;
	AttackTraceStartSocket = InStartSocket;
	AttackTraceEndSocket = InEndSocket;
}

void AGProjectCharacter::ResetAttackTraceSource()
{
	AttackTraceMesh = nullptr;
	AttackTraceStartSocket = NAME_None;
	AttackTraceEndSocket = NAME_None;
}

void AGProjectCharacter::SetActiveComboData(
	UGProjectComboData* NewGroundComboData,
	UGProjectComboData* NewAirComboData,
	UGProjectComboData* NewDashComboData)
{
	if (HasAuthority())
	{
		ActiveGroundComboData = NewGroundComboData;
		ActiveAirComboData = NewAirComboData;
		ActiveDashComboData = NewDashComboData;
	}
}

void AGProjectCharacter::ResetActiveComboData()
{
	if (HasAuthority())
	{
		ActiveGroundComboData = nullptr;
		ActiveAirComboData = nullptr;
		ActiveDashComboData = nullptr;
	}
}

void AGProjectCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AGProjectCharacter, ActiveGroundComboData);
	DOREPLIFETIME(AGProjectCharacter, ActiveAirComboData);
	DOREPLIFETIME(AGProjectCharacter, ActiveDashComboData);
}

void AGProjectCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);
	RefreshMovementStateTags();
}

void AGProjectCharacter::HandleDeath()
{
	if (bDead)
	{
		return;
	}

	bDead = true;

	if (UGProjectAbilitySystemComponent* ASC = GetGProjectAbilitySystemComponent())
	{
		ASC->AddLooseGameplayTag(GProjectGameplayTags::State_Character_Dead);
		ASC->CancelAllAbilities();
	}

	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

bool AGProjectCharacter::IsDead() const
{
	return bDead;
}

void AGProjectCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	InitAbilityActorInfo();
	AddCharacterAbilities();
}

void AGProjectCharacter::Jump()
{
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->GetCurrentAcceleration() = FVector::ZeroVector;
		
		FVector CurrentVelocity = GetCharacterMovement()->Velocity;
		CurrentVelocity.X = 0.35f;
		CurrentVelocity.Y = 0.35f;
		GetCharacterMovement()->Velocity = CurrentVelocity;
	}
	Super::Jump();
}

void AGProjectCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	if (GetCharacterMovement())
	{
		if (GetCharacterMovement()->MovementMode == MOVE_Falling)
		{
			FVector CurrentInput = GetCharacterMovement()->GetCurrentAcceleration();
			
			if (!CurrentInput.IsNearlyZero())
			{
				GetCharacterMovement()->MaxWalkSpeed = 600.0f; 
			}
			else
			{
				GetCharacterMovement()->MaxWalkSpeed = 300.0f; 
			}
		}
		else if (GetCharacterMovement()->MovementMode == MOVE_Walking)
		{
			GetCharacterMovement()->MaxWalkSpeed = 600.0f; 
		}
	}
}
	
void AGProjectCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	InitAbilityActorInfo();
}

void AGProjectCharacter::InitAbilityActorInfo()
{
	AGProjectPlayerState* GProjectPlayerState = GetPlayerState<AGProjectPlayerState>();
	if (!GProjectPlayerState)
	{
		return;
	}

	UGProjectAbilitySystemComponent* ASC = GProjectPlayerState->GetGProjectAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	ASC->InitAbilityActorInfo(GProjectPlayerState, this);
	RefreshMovementStateTags();
}

void AGProjectCharacter::RefreshMovementStateTags()
{
	UGProjectAbilitySystemComponent* ASC = GetGProjectAbilitySystemComponent();
	if (!ASC || !GetCharacterMovement())
	{
		return;
	}

	const bool bAirborne = GetCharacterMovement()->IsFalling();
	ASC->SetLooseGameplayTagCount(GProjectGameplayTags::State_Movement_Airborne, bAirborne ? 1 : 0);
	if (!bAirborne)
	{
		ASC->SetLooseGameplayTagCount(GProjectGameplayTags::State_Combat_AirAttackUsed, 0);
	}
}

void AGProjectCharacter::AddCharacterAbilities()
{
	if (!HasAuthority())
	{
		return;
	}

	UGProjectAbilitySystemComponent* ASC = GetGProjectAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	for (TSubclassOf<UGProjectGameplayAbility> AbilityClass : StartupAbilities)
	{
		if (!AbilityClass)
		{
			continue;
		}

		bool bAlreadyGranted = false;
		for (const FGameplayAbilitySpec& AbilitySpec : ASC->GetActivatableAbilities())
		{
			if (AbilitySpec.Ability && AbilitySpec.Ability->GetClass() == AbilityClass)
			{
				bAlreadyGranted = true;
				break;
			}
		}


		if (bAlreadyGranted)
		{
			continue;
		}

		FGameplayAbilitySpec AbilitySpec(AbilityClass, 1);
		ASC->GiveAbility(AbilitySpec);
	}
}