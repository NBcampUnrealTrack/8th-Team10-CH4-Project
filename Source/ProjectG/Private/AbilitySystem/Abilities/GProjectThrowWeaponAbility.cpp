#include "AbilitySystem/Abilities/GProjectThrowWeaponAbility.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemComponent.h"
#include "Character/GProjectCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GProjectGameplayTags.h"
#include "Item/GProjectItemActorBase.h"
#include "Item/GProjectItemHolderComponent.h"
#include "Item/Weapon/GProjectThrownWeapon.h"
#include "GameFramework/CharacterMovementComponent.h"

UGProjectThrowWeaponAbility::UGProjectThrowWeaponAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(GProjectGameplayTags::Ability_Combat_ThrowWeapon);
	SetAssetTags(AssetTags);

	StartupInputTag = GProjectGameplayTags::InputTag_Combat_ThrowWeapon;

	ActivationOwnedTags.AddTag(GProjectGameplayTags::State_Combat_Attacking);
	ActivationBlockedTags.AddTag(GProjectGameplayTags::State_Character_Dead);
	ActivationBlockedTags.AddTag(GProjectGameplayTags::State_Combat_Hitstun);
	ActivationBlockedTags.AddTag(GProjectGameplayTags::State_Combat_Knockdown);
}

void UGProjectThrowWeaponAbility::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	AGProjectCharacter* Character = Cast<AGProjectCharacter>(GetAvatarActorFromActorInfo());
	if (!Character || !ThrowMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const UGProjectItemHolderComponent* ItemHolder =
		Character->GetItemHolderComponent();
	if (!ItemHolder || !ItemHolder->GetHeldItem() || !ItemHolder->GetHeldItem()->CanBeThrown())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	EventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		GProjectGameplayTags::Event_Combat_Throw_Release,
		nullptr,
		true,
		true
	);
	EventTask->EventReceived.AddDynamic(this, &ThisClass::OnThrowReleaseEvent);
	EventTask->ReadyForActivation();

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		ThrowMontage
	);
	MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageEnded);
	MontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageEnded);
	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageEnded);
	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageEnded);
	MontageTask->ReadyForActivation();

	if (UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement())
	{
		if (MovementComponent->IsMovingOnGround())
		{
			MovementComponent->DisableMovement();

		}
	}
}

void UGProjectThrowWeaponAbility::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	if (ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
	{
		if (UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement())
		{
			if (MovementComponent->MovementMode == MOVE_None)
			{
				MovementComponent->SetMovementMode(MOVE_Walking);

			}
		}
	}

	MontageTask = nullptr;
	EventTask = nullptr;
	bWeaponThrown = false;

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGProjectThrowWeaponAbility::OnThrowReleaseEvent(FGameplayEventData Payload)
{
	const AActor* Avatar = GetAvatarActorFromActorInfo();
	if (bWeaponThrown || !Avatar || !Avatar->HasAuthority())
	{
		return;
	}

	bWeaponThrown = true;
	ThrowHeldWeapon();
}

void UGProjectThrowWeaponAbility::ThrowHeldWeapon()
{
	AGProjectCharacter* Character = Cast<AGProjectCharacter>(GetAvatarActorFromActorInfo());
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	UWorld* World = GetWorld();
	if (!Character || !SourceASC || !World)
	{
		return;
	}

	UGProjectItemHolderComponent* ItemHolder =
		Character->GetItemHolderComponent();
	if (!ItemHolder || !ItemHolder->GetHeldItem() || !ItemHolder->GetHeldItem()->CanBeThrown())
	{
		return;
	}

	AGProjectItemActorBase* ThrownItem = ItemHolder->ReleaseHeldItem();
	if (!ThrownItem)
	{
		return;
	}

	const USkeletalMeshComponent* CharacterMesh = Character->GetMesh();
	const FVector SocketLocation = CharacterMesh && !ThrowSocketName.IsNone()
		? CharacterMesh->GetSocketLocation(ThrowSocketName)
		: Character->GetActorLocation();

	FVector ThrowDirection = Character->GetActorForwardVector();
	ThrowDirection.Z = 0.0f;
	if (!ThrowDirection.Normalize())
	{
		ThrowDirection = Character->GetActorRotation().Vector();
		ThrowDirection.Z = 0.0f;
		ThrowDirection.Normalize();
	}

	const FRotator LaunchRotation = ThrowDirection.Rotation();
	const FVector SpawnLocation = SocketLocation + ThrowDirection * ThrowSpawnForwardOffset;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Character;
	SpawnParams.Instigator = Character;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AGProjectThrownWeapon* ThrownWeapon = World->SpawnActor<AGProjectThrownWeapon>(
		AGProjectThrownWeapon::StaticClass(),
		SpawnLocation,
		LaunchRotation,
		SpawnParams);
	if (!ThrownWeapon)
	{
		ThrownItem->SetActorLocation(SpawnLocation);
		ThrownItem->SetPickupEnabled(true);
		ThrownItem->ForceNetUpdate();
		return;
	}

	FGProjectDamageEffectParams LaunchDamageParams = ThrowDamageParams;
	LaunchDamageParams.SourceAbilitySystemComponent = SourceASC;

	ThrownWeapon->InitAndLaunch(
		ThrownItem,
		ThrowDirection * ThrowSpeed,
		LaunchDamageParams,
		DamageGameplayEffectClass,
		HitstunGameplayEffectClass,
		ThrowGravityScale,
		MaxFlightTime);
	ThrownWeapon->ForceNetUpdate();
	ThrownItem->OnThrowStarted(Character);
}

void UGProjectThrowWeaponAbility::OnMontageEnded()
{
	const AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!bWeaponThrown && Avatar && Avatar->HasAuthority())
	{
		bWeaponThrown = true;
		ThrowHeldWeapon();
	}

	FinishAbility();
}

void UGProjectThrowWeaponAbility::FinishAbility()
{
	if (IsActive())
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}
