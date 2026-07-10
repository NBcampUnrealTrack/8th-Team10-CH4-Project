// Copyright Epic Games, Inc. All Rights Reserved.

#include "Targeting/GProjectLockOnComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/GProjectAbilitySystemLibrary.h"
#include "CollisionQueryParams.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GProjectGameplayTags.h"
#include "Net/UnrealNetwork.h"
#include "Components/WidgetComponent.h"

UGProjectLockOnComponent::UGProjectLockOnComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);

	LockOnWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("LockOnWidget"));
	LockOnWidgetComp->SetVisibility(false);
	LockOnWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
}

void UGProjectLockOnComponent::BeginPlay()
{
	Super::BeginPlay();

	if (LockOnWidgetComp && LockOnWidgetClass)
	{
		LockOnWidgetComp->SetWidgetClass(LockOnWidgetClass);
	}

	if (const ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		bDefaultOrientRotationToMovement = Character->GetCharacterMovement()->bOrientRotationToMovement;
	}
}

void UGProjectLockOnComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UGProjectLockOnComponent, CurrentTarget);
}

void UGProjectLockOnComponent::StartLockOn()
{
	if (!GetOwner() || !GetWorld())
	{
		return;
	}

	if (CurrentTarget)
	{
		return;
	}

	AActor* RequestedTarget = FindBestTarget();
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		SetCurrentTarget(IsValidTarget(RequestedTarget) ? RequestedTarget : nullptr);
		return;
	}

	ServerSetLockOnTarget(RequestedTarget);
}

void UGProjectLockOnComponent::ClearLockOn()
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		SetCurrentTarget(nullptr);
		return;
	}

	ServerSetLockOnTarget(nullptr);
}

void UGProjectLockOnComponent::ServerSetLockOnTarget_Implementation(AActor* NewTarget)
{
	const bool bValidTarget = IsValidTarget(NewTarget);
	SetCurrentTarget(bValidTarget ? NewTarget : nullptr);
}

void UGProjectLockOnComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!CurrentTarget)
	{
		return;
	}

	if (GetOwner() && GetOwner()->HasAuthority() && !IsValidTarget(CurrentTarget))
	{
		SetCurrentTarget(nullptr);
		return;
	}

	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if ((GetOwner() && GetOwner()->HasAuthority()) || (OwnerPawn && OwnerPawn->IsLocallyControlled()))
	{
		UpdateOwnerRotation(DeltaTime);
	}
}

AActor* UGProjectLockOnComponent::FindBestTarget() const
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor || !GetWorld())
	{
		return nullptr;
	}

	TArray<AActor*> Candidates;
	TArray<AActor*> ActorsToIgnore{ OwnerActor };
	UGProjectAbilitySystemLibrary::GetLivePlayersWithinRadius(
		OwnerActor,
		Candidates,
		ActorsToIgnore,
		LockOnRadius,
		OwnerActor->GetActorLocation());

	AActor* BestTarget = nullptr;
	float BestScore = TNumericLimits<float>::Max();
	FVector OwnerForward = OwnerActor->GetActorForwardVector();
	OwnerForward.Z = 0.0f;
	OwnerForward.Normalize();

	for (AActor* Candidate : Candidates)
	{
		if (!IsValidTarget(Candidate))
		{
			continue;
		}

		FVector ToCandidate = Candidate->GetActorLocation() - OwnerActor->GetActorLocation();
		ToCandidate.Z = 0.0f;
		const float Distance = ToCandidate.Size();
		if (!ToCandidate.Normalize())
		{
			continue;
		}

		const float DirectionPenalty = 1.0f - FVector::DotProduct(OwnerForward, ToCandidate);
		const float Score = Distance + DirectionPenalty * LockOnRadius * 0.5f;
		if (Score < BestScore)
		{
			BestScore = Score;
			BestTarget = Candidate;
		}
	}

	return BestTarget;
}

bool UGProjectLockOnComponent::IsValidTarget(AActor* Target) const
{
	const AActor* OwnerActor = GetOwner();
	if (!OwnerActor || !Target || Target == OwnerActor)
	{
		return false;
	}

	if (FVector::DistSquared2D(OwnerActor->GetActorLocation(), Target->GetActorLocation()) > FMath::Square(BreakDistance))
	{
		return false;
	}

	const UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
	return TargetASC &&
		!TargetASC->HasMatchingGameplayTag(GProjectGameplayTags::State_Character_Dead) &&
		HasLineOfSightTo(Target);
}

bool UGProjectLockOnComponent::HasLineOfSightTo(const AActor* Target) const
{
	const AActor* OwnerActor = GetOwner();
	const UWorld* World = GetWorld();
	if (!OwnerActor || !Target || !World)
	{
		return false;
	}

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(LockOnVisibility), false, OwnerActor);
	FHitResult HitResult;
	const bool bBlocked = World->LineTraceSingleByChannel(
		HitResult,
		OwnerActor->GetActorLocation(),
		Target->GetActorLocation(),
		VisibilityTraceChannel,
		QueryParams);

	return !bBlocked || HitResult.GetActor() == Target;
}

void UGProjectLockOnComponent::SetCurrentTarget(AActor* NewTarget)
{
	if (CurrentTarget == NewTarget)
	{
		return;
	}

	CurrentTarget = NewTarget;
	ApplyLockOnState();
	OnLockOnTargetChanged.Broadcast(CurrentTarget);
	UpdateLockOnWidget();

	if (AActor* OwnerActor = GetOwner())
	{
		OwnerActor->ForceNetUpdate();
	}
}

void UGProjectLockOnComponent::OnRep_CurrentTarget()
{
	ApplyLockOnState();
	OnLockOnTargetChanged.Broadcast(CurrentTarget);
	UpdateLockOnWidget();
}

void UGProjectLockOnComponent::ApplyLockOnState()
{
	if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		Character->GetCharacterMovement()->bOrientRotationToMovement = CurrentTarget
			? false
			: bDefaultOrientRotationToMovement;
	}
}

void UGProjectLockOnComponent::UpdateOwnerRotation(float DeltaTime) const
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor || !CurrentTarget)
	{
		return;
	}

	FVector ToTarget = CurrentTarget->GetActorLocation() - OwnerActor->GetActorLocation();
	ToTarget.Z = 0.0f;
	if (ToTarget.IsNearlyZero())
	{
		return;
	}

	const FRotator DesiredRotation(0.0f, ToTarget.Rotation().Yaw, 0.0f);
	const FRotator NewRotation = FMath::RInterpTo(
		OwnerActor->GetActorRotation(),
		DesiredRotation,
		DeltaTime,
		RotationInterpSpeed);
	OwnerActor->SetActorRotation(NewRotation);
}

void UGProjectLockOnComponent::UpdateLockOnWidget()
{
	if (!LockOnWidgetComp)
	{
		return;
	}

	const ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character || !Character->IsLocallyControlled())
	{
		return;
	}

	if (CurrentTarget)
	{
		if (const ACharacter* TargetCharacter = Cast<ACharacter>(CurrentTarget))
		{
			if (USkeletalMeshComponent* TargetMesh = TargetCharacter->GetMesh())
			{
				LockOnWidgetComp->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

				LockOnWidgetComp->AttachToComponent(TargetMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("pelvis"));
				LockOnWidgetComp->SetVisibility(true);
			}
		}
	}
	else
	{
		LockOnWidgetComp->SetVisibility(false);
		if (Character->GetRootComponent())
		{
			LockOnWidgetComp->AttachToComponent(Character->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		}
	}
}