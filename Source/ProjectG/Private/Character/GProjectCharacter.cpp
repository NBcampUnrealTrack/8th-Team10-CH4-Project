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
#include "GameplayEffect.h"
#include "GProjectGameplayTags.h"
#include "Item/GProjectItemHolderComponent.h"
#include "Player/GProjectPlayerState.h"
#include "Targeting/GProjectLockOnComponent.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystem/GProjectAttributeSet.h"

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
	ItemHolderComponent = CreateDefaultSubobject<UGProjectItemHolderComponent>(TEXT("ItemHolderComponent"));
}

void AGProjectCharacter::BeginPlay()
{
	Super::BeginPlay();
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void AGProjectCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	RefreshMovementStateTags();
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

UGProjectItemHolderComponent* AGProjectCharacter::GetItemHolderComponent() const
{
	return FindComponentByClass<UGProjectItemHolderComponent>();
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

void AGProjectCharacter::ResetForNewRound(const FTransform& SpawnTransform)
{
	if (!HasAuthority())
	{
		return;
	}

	UGProjectAbilitySystemComponent* ASC = GetGProjectAbilitySystemComponent();

	AGProjectPlayerState* PS = GetPlayerState<AGProjectPlayerState>();

	UGProjectAttributeSet* AttributeSet = PS ? PS->GetAttributeSet() : nullptr;

	if (ASC)
	{
		ASC->CancelAllAbilities();

		FGameplayTagContainer EffectTagsToRemove;

		EffectTagsToRemove.AddTag(GProjectGameplayTags::State_Combat_Hitstun);

		ASC->RemoveActiveEffectsWithGrantedTags(EffectTagsToRemove);

		ASC->SetLooseGameplayTagCount(GProjectGameplayTags::State_Character_Dead, 0);

		ASC->SetLooseGameplayTagCount(GProjectGameplayTags::State_Combat_AirAttackUsed, 0);

		ASC->SetLooseGameplayTagCount(GProjectGameplayTags::State_Movement_Airborne, 0);
	}

	if (LockOnComponent)
	{
		LockOnComponent->ClearLockOn();
	}

	StopJumping();

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
		Movement->ClearAccumulatedForces();
	}

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	TeleportTo(SpawnTransform.GetLocation(), SpawnTransform.Rotator(), false, true);

	bDead = false;

	if (ASC && AttributeSet)
	{
		ASC->SetNumericAttributeBase(
			UGProjectAttributeSet::GetHealthAttribute(),
			AttributeSet->GetMaxHealth()
		);

		ASC->SetNumericAttributeBase(
			UGProjectAttributeSet::GetSPAttribute(),
			AttributeSet->GetMaxSP()
		);
	}

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->SetMovementMode(MOVE_Walking);
	}

	RefreshMovementStateTags();

	ForceNetUpdate();
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
	SetSprintRequested(false);

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

void AGProjectCharacter::StartSprint()
{
	SetSprintRequested(true);
}

void AGProjectCharacter::StopSprint()
{
	SetSprintRequested(false);
}

void AGProjectCharacter::SetSprintRequested(bool bRequested)
{
	bSprintRequested = bRequested;
	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->MaxWalkSpeed = bSprintRequested ? SprintSpeed : WalkSpeed;
	}
	RefreshMovementStateTags();
}

void AGProjectCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	InitAbilityActorInfo();
	AddCharacterAbilities();
	ApplySPRegenEffect();
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

void AGProjectCharacter::ApplySPRegenEffect()
{
	UGProjectAbilitySystemComponent* ASC = GetGProjectAbilitySystemComponent();
	if (!HasAuthority() || !ASC || !SPRegenGameplayEffectClass)
	{
		return;
	}

	FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
	EffectContext.AddSourceObject(this);
	ASC->ApplyGameplayEffectToSelf(
		SPRegenGameplayEffectClass->GetDefaultObject<UGameplayEffect>(),
		1.0f,
		EffectContext);
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

	const bool bDashing = bSprintRequested &&
		!bAirborne &&
		GetVelocity().SizeSquared2D() >= FMath::Square(DashingSpeedThreshold);
	ASC->SetLooseGameplayTagCount(GProjectGameplayTags::State_Movement_Dashing, bDashing ? 1 : 0);
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
