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
	CagePhysicsRoot->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CagePhysicsRoot->SetCollisionObjectType(ECC_WorldDynamic);
	CagePhysicsRoot->SetCollisionResponseToAllChannels(ECR_Block);
	CagePhysicsRoot->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	CagePhysicsRoot->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	CagePhysicsRoot->SetSimulatePhysics(false);
	CagePhysicsRoot->SetEnableGravity(true);
	CagePhysicsRoot->SetIsReplicated(true);

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
	BackWallCollision->SetCollisionProfileName(TEXT("BlockAll"));

	LeftWallCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftWallCollision"));
	LeftWallCollision->SetupAttachment(CagePhysicsRoot);
	LeftWallCollision->SetCollisionProfileName(TEXT("BlockAll"));

	RightWallCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("RightWallCollision"));
	RightWallCollision->SetupAttachment(CagePhysicsRoot);
	RightWallCollision->SetCollisionProfileName(TEXT("BlockAll"));

	FloorCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("FloorCollision"));
	FloorCollision->SetupAttachment(CagePhysicsRoot);
	FloorCollision->SetCollisionProfileName(TEXT("BlockAll"));

	CeilingCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("CeilingCollision"));
	CeilingCollision->SetupAttachment(CagePhysicsRoot);
	CeilingCollision->SetCollisionProfileName(TEXT("BlockAll"));

	DoorCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("DoorCollision"));
	DoorCollision->SetupAttachment(DoorPivot);
	DoorCollision->SetCollisionProfileName(TEXT("BlockAll"));
}

void AGProjectCageActor::BeginPlay()
{
	Super::BeginPlay();

	if (DoorPivot)
	{
		ClosedDoorPivotRotation = DoorPivot->GetRelativeRotation();
	}

	ApplyDoorCollisionState();

	if (bSimulatePhysicsFromStart && CagePhysicsRoot)
	{
		CagePhysicsRoot->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		CagePhysicsRoot->SetCollisionObjectType(ECC_WorldDynamic);
		CagePhysicsRoot->SetCollisionResponseToAllChannels(ECR_Block);
		CagePhysicsRoot->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
		CagePhysicsRoot->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
		CagePhysicsRoot->SetEnableGravity(true);

		if (HasAuthority())
		{
			CagePhysicsRoot->SetSimulatePhysics(true);
			CagePhysicsRoot->WakeAllRigidBodies();
		}
	}

	if (HasAuthority() && bOpenByTimer)
	{
		if (AutoOpenDelay <= 0.0f)
		{
			OpenDoor();
		}
		else
		{
			GetWorldTimerManager().SetTimer(
				AutoOpenTimerHandle,
				this,
				&ThisClass::HandleAutoOpenTimer,
				AutoOpenDelay,
				false
			);
		}
	}
}

void AGProjectCageActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const float TargetRoll = GetTargetDoorRoll();

	const FRotator CurrentRotation = DoorPivot->GetRelativeRotation();
	const FRotator TargetRotation = FRotator(0.0f, 0.0f, TargetRoll);

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

void AGProjectCageActor::HandleAutoOpenTimer()
{
	OpenDoor();
}

void AGProjectCageActor::OnRep_IsOpen()
{
	ApplyDoorCollisionState();
}

float AGProjectCageActor::GetTargetDoorRoll() const
{
	return bIsOpen ? OpenRoll : 0.0f;
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
		DoorCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
}

void AGProjectCageActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGProjectCageActor, bIsOpen);

}