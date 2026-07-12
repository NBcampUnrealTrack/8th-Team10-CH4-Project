// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/GProjectCharacter.h"

#include "AbilitySystem/GProjectAbilitySystemComponent.h"
#include "AbilitySystem/Abilities/GProjectGameplayAbility.h"
#include "AbilitySystem/Combo/GProjectComboData.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/MeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameplayEffect.h"
#include "GProjectGameplayTags.h"
#include "Item/GProjectItemHolderComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Player/GProjectPlayerState.h"
#include "Targeting/GProjectLockOnComponent.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystem/GProjectAttributeSet.h"
#include "TimerManager.h"
#include "Game/GProjectGameMode.h"
#include "Player/GProjectPlayerColors.h"

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
		GetCharacterMovement()->AirControl = 0.95f; 
		GetCharacterMovement()->FallingLateralFriction = 0.5f; 
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

void AGProjectCharacter::ApplyPlayerColor(int32 ColorIndex)
{
	USkeletalMeshComponent* CharacterMesh = GetMesh();
	if (!CharacterMesh)
	{
		return;
	}

	const FLinearColor PlayerColor = GProjectPlayerColors::GetColor(ColorIndex);

	PlayerColorMaterials.Reset();

	const int32 MaterialCount = CharacterMesh->GetNumMaterials();

	for (int32 Index = 0; Index < MaterialCount; ++Index)
	{
		UMaterialInstanceDynamic* MID = CharacterMesh->CreateDynamicMaterialInstance(Index);
		if (!MID)
		{
			continue;
		}

		MID->SetVectorParameterValue(TEXT("PlayerColor"), PlayerColor);

		PlayerColorMaterials.Add(MID);
	}
}

void AGProjectCharacter::PlayHitFlash()
{
	if (HasAuthority())
	{
		MulticastPlayHitFlash();
		return;
	}

	MulticastPlayHitFlash();
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

		ASC->SetLooseGameplayTagCount(GProjectGameplayTags::State_Combat_Airborne, 0);
	}

	SetHitFlashAmount(0.0f);
	GetWorldTimerManager().ClearTimer(HitFlashTimer);

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

	GetCapsuleComponent()->SetCollisionEnabled(
		ECollisionEnabled::NoCollision
	);

	GetCapsuleComponent()->SetCollisionEnabled(
		ECollisionEnabled::NoCollision
	);

	TeleportTo(
		SpawnTransform.GetLocation(),
		SpawnTransform.Rotator(),
		false,
		true
	);

	bDead = false;

	MulticastResetDeathState();

	SetActorHiddenInGame(false);

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

EGProjectCombatStyle AGProjectCharacter::GetCombatStyle() const
{
	return CombatStyle;
}

void AGProjectCharacter::SetCombatStyle(EGProjectCombatStyle NewCombatStyle)
{
	if (HasAuthority())
	{
		CombatStyle = NewCombatStyle;
	}

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
	DOREPLIFETIME(AGProjectCharacter, CombatStyle);
}

void AGProjectCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);
	RefreshMovementStateTags();
}

void AGProjectCharacter::HandleDeath()
{
	if (!HasAuthority() || bDead)
	{
		return;
	}

	bDead = true;
	SetSprintRequested(false);
	if (LockOnComponent)
	{
		LockOnComponent->ClearLockOn();
	}

	if (UGProjectAbilitySystemComponent* ASC = GetGProjectAbilitySystemComponent())
	{
		ASC->AddLooseGameplayTag(GProjectGameplayTags::State_Character_Dead);
		ASC->CancelAllAbilities();
	}

	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	MulticastPlayDeath();

	if (AGProjectGameMode* GM = Cast<AGProjectGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GM->NotifyPlayerDied(GetPlayerState<AGProjectPlayerState>());
	}
}

void AGProjectCharacter::MulticastPlayDeath_Implementation()
{
	bDead = true;

	UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (!AnimInstance || !DeathMontage || DeathMontagePlayRate <= 0.0f)
	{
		return;
	}

	AnimInstance->Montage_Play(DeathMontage, DeathMontagePlayRate);
	AnimInstance->Montage_JumpToSection(DeathFallSection, DeathMontage);
	AnimInstance->Montage_SetNextSection(DeathFallSection, DeathDownLoopSection, DeathMontage);
	AnimInstance->Montage_SetNextSection(DeathDownLoopSection, DeathDownLoopSection, DeathMontage);
}

void AGProjectCharacter::MulticastPlayHitFlash_Implementation()
{
	if (PlayerColorMaterials.IsEmpty() || HitFlashDuration <= 0.0f)
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(HitFlashTimer);
	SetHitFlashAmount(HitFlashAmount);
	GetWorldTimerManager().SetTimer(
		HitFlashTimer,
		this,
		&ThisClass::ResetHitFlash,
		HitFlashDuration,
		false);
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

	if (AGProjectPlayerState* PS = GetPlayerState<AGProjectPlayerState>())
	{
		ApplyPlayerColor(PS->GetPlayerColorIndex());
	}
}

void AGProjectCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	InitAbilityActorInfo();

	if (AGProjectPlayerState* PS = GetPlayerState<AGProjectPlayerState>())
	{
		ApplyPlayerColor(PS->GetPlayerColorIndex());
	}
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

void AGProjectCharacter::SetHitFlashAmount(float Amount)
{
	if (HitFlashParameterName.IsNone())
	{
		return;
	}

	for (UMaterialInstanceDynamic* DynamicMaterial : PlayerColorMaterials)
	{
		if (!DynamicMaterial)
		{
			continue;
		}

		DynamicMaterial->SetScalarParameterValue(HitFlashParameterName, Amount);
	}
}

void AGProjectCharacter::ResetHitFlash()
{
	SetHitFlashAmount(0.0f);
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
		ASC->SetLooseGameplayTagCount(GProjectGameplayTags::State_Combat_Airborne, 0);
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

void AGProjectCharacter::MulticastResetDeathState_Implementation()
{
	bDead = false;

	if (USkeletalMeshComponent* CharacterMesh = GetMesh())
	{
		CharacterMesh->SetHiddenInGame(false, true);
		CharacterMesh->SetVisibility(true, true);
		CharacterMesh->SetComponentTickEnabled(true);
		CharacterMesh->SetSimulatePhysics(false);

		if (UAnimInstance* AnimInstance =
			CharacterMesh->GetAnimInstance())
		{
			if (DeathMontage)
			{
				AnimInstance->Montage_Stop(
					0.0f,
					DeathMontage
				);
			}
		}
	}

	TArray<AActor*> AttachedActors;
	GetAttachedActors(AttachedActors);

	for (AActor* AttachedActor : AttachedActors)
	{
		if (AttachedActor)
		{
			AttachedActor->SetActorHiddenInGame(false);
		}
	}
}

