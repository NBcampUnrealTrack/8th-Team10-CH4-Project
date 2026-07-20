// Fill out your copyright notice in the Description page of Project Settings.

#include "Actor/AGProjectCageActor.h"

#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

AGProjectCageActor::AGProjectCageActor()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	SetReplicateMovement(true);

	CagePhysicsRoot = CreateDefaultSubobject<UBoxComponent>(TEXT("CagePhysicsRoot"));
	SetRootComponent(CagePhysicsRoot);

	CagePhysicsRoot->SetMobility(EComponentMobility::Movable);
	CagePhysicsRoot->SetBoxExtent(FVector(150.0f, 150.0f, 150.0f));
	CagePhysicsRoot->SetIsReplicated(true);
	CagePhysicsRoot->SetSimulatePhysics(false);
	CagePhysicsRoot->SetEnableGravity(true);

	SetupPhysicsRootCollision();

	CageFrameMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CageFrameMesh"));
	CageFrameMesh->SetupAttachment(CagePhysicsRoot);
	CageFrameMesh->SetMobility(EComponentMobility::Movable);
	CageFrameMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CageFrameMesh->SetSimulatePhysics(false);
	CageFrameMesh->SetEnableGravity(false);

	DoorPivot = CreateDefaultSubobject<USceneComponent>(TEXT("DoorPivot"));
	DoorPivot->SetupAttachment(CagePhysicsRoot);
	DoorPivot->SetMobility(EComponentMobility::Movable);

	CageDoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CageDoorMesh"));
	CageDoorMesh->SetupAttachment(DoorPivot);
	CageDoorMesh->SetMobility(EComponentMobility::Movable);
	CageDoorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CageDoorMesh->SetSimulatePhysics(false);
	CageDoorMesh->SetEnableGravity(false);

	BackWallCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BackWallCollision"));
	BackWallCollision->SetupAttachment(CagePhysicsRoot);
	SetupPawnBlockerCollision(BackWallCollision);

	LeftWallCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftWallCollision"));
	LeftWallCollision->SetupAttachment(CagePhysicsRoot);
	SetupPawnBlockerCollision(LeftWallCollision);

	RightWallCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("RightWallCollision"));
	RightWallCollision->SetupAttachment(CagePhysicsRoot);
	SetupPawnBlockerCollision(RightWallCollision);

	FloorCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("FloorCollision"));
	FloorCollision->SetupAttachment(CagePhysicsRoot);
	SetupPawnBlockerCollision(FloorCollision);

	CeilingCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("CeilingCollision"));
	CeilingCollision->SetupAttachment(CagePhysicsRoot);
	SetupPawnBlockerCollision(CeilingCollision);

	DoorCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("DoorCollision"));
	DoorCollision->SetupAttachment(DoorPivot);
	SetupPawnBlockerCollision(DoorCollision);
}

void AGProjectCageActor::BeginPlay()
{
	Super::BeginPlay();

	InitialActorTransform = GetActorTransform();

	if (DoorPivot)
	{
		InitialDoorPivotRotation = DoorPivot->GetRelativeRotation();
	}

	SetupPhysicsRootCollision();

	SetupPawnBlockerCollision(BackWallCollision);
	SetupPawnBlockerCollision(LeftWallCollision);
	SetupPawnBlockerCollision(RightWallCollision);
	SetupPawnBlockerCollision(FloorCollision);
	SetupPawnBlockerCollision(CeilingCollision);
	SetupPawnBlockerCollision(DoorCollision);

	ApplyDoorCollisionState();

	if (bSimulatePhysicsFromStart)
	{
		SetCagePhysicsEnabled(true);
	}

	if (HasAuthority())
	{
		StartAutoOpenTimer();
	}
}

void AGProjectCageActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!DoorPivot)
	{
		return;
	}

	FRotator TargetRotation = InitialDoorPivotRotation;

	if (bIsOpen)
	{
		TargetRotation.Roll += OpenRoll;
	}

	const FRotator CurrentRotation = DoorPivot->GetRelativeRotation();

	const FRotator NewRotation = FMath::RInterpTo(
		CurrentRotation,
		TargetRotation,
		DeltaSeconds,
		DoorInterpSpeed
	);

	DoorPivot->SetRelativeRotation(NewRotation);
}

void AGProjectCageActor::OpenDoor()
{
	if (!HasAuthority())
	{
		return;
	}

	if (bIsOpen)
	{
		return;
	}

	bIsOpen = true;
	ApplyDoorCollisionState();

	ForceNetUpdate();
}

void AGProjectCageActor::CloseDoor()
{
	if (!HasAuthority())
	{
		return;
	}

	if (!bIsOpen)
	{
		return;
	}

	bIsOpen = false;
	ApplyDoorCollisionState();

	ForceNetUpdate();
}

void AGProjectCageActor::ToggleDoor()
{
	if (!HasAuthority())
	{
		return;
	}

	if (bIsOpen)
	{
		CloseDoor();
	}
	else
	{
		OpenDoor();
	}
}

void AGProjectCageActor::StartCollapse()
{
	if (!HasAuthority())
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(AutoOpenTimerHandle);

	SetCagePhysicsEnabled(true);

	if (CagePhysicsRoot)
	{
		CagePhysicsRoot->WakeAllRigidBodies();

		if (CollapseImpulseStrength > 0.0f)
		{
			CagePhysicsRoot->AddImpulse(FVector::DownVector * CollapseImpulseStrength, NAME_None, true);
		}
	}

	ForceNetUpdate();
}

void AGProjectCageActor::ResetCageForNewRound()
{
	if (!HasAuthority())
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(AutoOpenTimerHandle);

	bIsOpen = false;

	SetCagePhysicsEnabled(false);

	SetActorTransform(
		InitialActorTransform,
		false,
		nullptr,
		ETeleportType::TeleportPhysics
	);

	if (DoorPivot)
	{
		DoorPivot->SetRelativeRotation(InitialDoorPivotRotation);
	}

	SetupPhysicsRootCollision();

	SetupPawnBlockerCollision(BackWallCollision);
	SetupPawnBlockerCollision(LeftWallCollision);
	SetupPawnBlockerCollision(RightWallCollision);
	SetupPawnBlockerCollision(FloorCollision);
	SetupPawnBlockerCollision(CeilingCollision);
	SetupPawnBlockerCollision(DoorCollision);

	ApplyDoorCollisionState();

	if (bSimulatePhysicsFromStart)
	{
		SetCagePhysicsEnabled(true);
	}

	StartAutoOpenTimer();

	ForceNetUpdate();
}

void AGProjectCageActor::HandleAutoOpenTimer()
{
	OpenDoor();
}

void AGProjectCageActor::StartAutoOpenTimer()
{
	if (!HasAuthority())
	{
		return;
	}

	if (!bOpenByTimer)
	{
		return;
	}

	if (AutoOpenDelay <= 0.0f)
	{
		OpenDoor();
		return;
	}

	GetWorldTimerManager().SetTimer(
		AutoOpenTimerHandle,
		this,
		&ThisClass::HandleAutoOpenTimer,
		AutoOpenDelay,
		false
	);
}

void AGProjectCageActor::OnRep_IsOpen()
{
	ApplyDoorCollisionState();
}

void AGProjectCageActor::ApplyDoorCollisionState()
{
	if (!DoorCollision)
	{
		return;
	}

	if (bIsOpen)
	{
		DoorCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	else
	{
		SetupPawnBlockerCollision(DoorCollision);
	}
}

void AGProjectCageActor::SetupPhysicsRootCollision()
{
	if (!CagePhysicsRoot)
	{
		return;
	}

	CagePhysicsRoot->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CagePhysicsRoot->SetCollisionObjectType(ECC_WorldDynamic);

	CagePhysicsRoot->SetCollisionResponseToAllChannels(ECR_Block);
	CagePhysicsRoot->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	CagePhysicsRoot->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
}

void AGProjectCageActor::SetupPawnBlockerCollision(UBoxComponent* CollisionBox)
{
	if (!CollisionBox)
	{
		return;
	}

	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionBox->SetCollisionObjectType(ECC_WorldDynamic);

	CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
}

void AGProjectCageActor::SetCagePhysicsEnabled(bool bEnable)
{
	if (!CagePhysicsRoot)
	{
		return;
	}

	SetupPhysicsRootCollision();

	CagePhysicsRoot->SetEnableGravity(true);
	CagePhysicsRoot->SetLinearDamping(LinearDamping);
	CagePhysicsRoot->SetAngularDamping(AngularDamping);
	CagePhysicsRoot->SetMassOverrideInKg(NAME_None, PhysicsMassKg, true);

	LockPhysicsRotation();

	if (!HasAuthority())
	{
		return;
	}

	if (!bEnable)
	{
		if (CagePhysicsRoot->IsSimulatingPhysics())
		{
			CagePhysicsRoot->SetPhysicsLinearVelocity(FVector::ZeroVector);
			CagePhysicsRoot->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
		}

		CagePhysicsRoot->SetSimulatePhysics(false);
		return;
	}

	CagePhysicsRoot->SetSimulatePhysics(true);
	CagePhysicsRoot->WakeAllRigidBodies();
}

void AGProjectCageActor::LockPhysicsRotation()
{
	if (!CagePhysicsRoot)
	{
		return;
	}

	if (!bLockCageRotation)
	{
		return;
	}

	CagePhysicsRoot->BodyInstance.bLockXRotation = true;
	CagePhysicsRoot->BodyInstance.bLockYRotation = true;
	CagePhysicsRoot->BodyInstance.bLockZRotation = true;

	CagePhysicsRoot->SetAngularDamping(AngularDamping);
}

void AGProjectCageActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGProjectCageActor, bIsOpen);
}