#include "Actor/GCageDoorActor.h"

#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

AGCageDoorActor::AGCageDoorActor()
{
	PrimaryActorTick.bCanEverTick = true;

	FrameMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CageFrame"));
	SetRootComponent(FrameMesh);
	FrameMesh->SetMobility(EComponentMobility::Static);
	FrameMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	DoorHinge = CreateDefaultSubobject<USceneComponent>(TEXT("DoorHinge"));
	DoorHinge->SetupAttachment(FrameMesh);
	DoorHinge->SetRelativeLocation(FVector(100.684f, -102.904f, 0.0f));

	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CageDoor"));
	DoorMesh->SetupAttachment(DoorHinge);
	DoorMesh->SetRelativeLocation(FVector(-100.684f, 102.904f, 0.0f));
	DoorMesh->SetMobility(EComponentMobility::Movable);
	DoorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DoorMesh->OnClicked.AddDynamic(this, &ThisClass::HandleDoorClicked);

	auto SetupFrameCollision = [this](UBoxComponent* Box, const FVector& Location, const FVector& Extent)
	{
		Box->SetupAttachment(FrameMesh);
		Box->SetRelativeLocation(Location);
		Box->SetBoxExtent(Extent);
		Box->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	};

	BackWallCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BackWallCollision"));
	SetupFrameCollision(BackWallCollision, FVector(-95.0f, -2.2f, 104.0f), FVector(6.0f, 100.7f, 100.7f));

	LeftWallCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftWallCollision"));
	SetupFrameCollision(LeftWallCollision, FVector(0.0f, -96.9f, 104.0f), FVector(95.0f, 6.0f, 100.7f));

	RightWallCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("RightWallCollision"));
	SetupFrameCollision(RightWallCollision, FVector(0.0f, 92.5f, 104.0f), FVector(95.0f, 6.0f, 100.7f));

	FloorCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("FloorCollision"));
	SetupFrameCollision(FloorCollision, FVector(0.0f, -2.2f, 8.0f), FVector(100.7f, 100.7f, 4.0f));

	CeilingCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("CeilingCollision"));
	SetupFrameCollision(CeilingCollision, FVector(0.0f, -2.2f, 200.5f), FVector(100.7f, 100.7f, 4.0f));

	DoorCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("DoorCollision"));
	// Keep collision attached to the visible door itself. This ensures that any
	// Blueprint/editor offset applied to DoorMesh also moves the blocking shape.
	DoorCollision->SetupAttachment(DoorMesh);
	DoorCollision->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	DoorCollision->SetMobility(EComponentMobility::Movable);
	DoorCollision->OnClicked.AddDynamic(this, &ThisClass::HandleDoorClicked);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> FrameAsset(
		TEXT("/Game/Developers/ChangWuk/Cage/Cage1_Frame.Cage1_Frame"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> DoorAsset(
		TEXT("/Game/Developers/ChangWuk/Cage/Cage1_Door.Cage1_Door"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> CageMaterial(
		TEXT("/Game/Developers/ChangWuk/Cage/Material.Material"));

	if (FrameAsset.Succeeded())
	{
		FrameMesh->SetStaticMesh(FrameAsset.Object);
	}
	if (DoorAsset.Succeeded())
	{
		DoorMesh->SetStaticMesh(DoorAsset.Object);
		const FBoxSphereBounds DoorBounds = DoorAsset.Object->GetBounds();
		const float DoorOuterX = DoorBounds.Origin.X + DoorBounds.BoxExtent.X;
		const float DoorEdgeY = DoorBounds.Origin.Y - DoorBounds.BoxExtent.Y;
		float FrameEdgeY = DoorEdgeY;
		if (FrameAsset.Succeeded())
		{
			const FBoxSphereBounds FrameBounds = FrameAsset.Object->GetBounds();
			FrameEdgeY = FrameBounds.Origin.Y - FrameBounds.BoxExtent.Y;
		}

		// The hinge lives on the frame corner. Offset the mesh by its own local
		// edge so that the visible door edge lands exactly on that hinge.
		DoorHinge->SetRelativeLocation(FVector(DoorOuterX, FrameEdgeY, 0.0f));
		DoorMesh->SetRelativeLocation(FVector(-DoorOuterX, -DoorEdgeY, 0.0f));
		DoorCollision->SetRelativeLocation(DoorBounds.Origin);
		DoorCollision->SetBoxExtent(DoorBounds.BoxExtent);
	}
	if (CageMaterial.Succeeded())
	{
		FrameMesh->SetMaterial(0, CageMaterial.Object);
		DoorMesh->SetMaterial(0, CageMaterial.Object);
	}
}

void AGCageDoorActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	SynchronizeDoorCollision();
}

void AGCageDoorActor::BeginPlay()
{
	Super::BeginPlay();
	SynchronizeDoorCollision();
	bTargetOpen = bStartOpen;
	if (bStartOpen)
	{
		DoorHinge->SetRelativeRotation(FRotator(0.0f, OpenAngle, 0.0f));
	}
}

void AGCageDoorActor::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	SynchronizeDoorCollision();

	const float TargetYaw = bTargetOpen ? OpenAngle : 0.0f;
	const float CurrentYaw = DoorHinge->GetRelativeRotation().Yaw;
	const float NewYaw = FMath::FInterpConstantTo(CurrentYaw, TargetYaw, DeltaSeconds, OpenSpeed);
	DoorHinge->SetRelativeRotation(FRotator(0.0f, NewYaw, 0.0f));

	// Once the door has swung clear of the opening, its broad box collision is
	// no longer useful and can catch a character capsule near the hinge.
	const bool bDoorClearOfEntrance = bTargetOpen && FMath::Abs(NewYaw) >= 70.0f;
	DoorCollision->SetCollisionEnabled(
		bDoorClearOfEntrance ? ECollisionEnabled::NoCollision : ECollisionEnabled::QueryAndPhysics);
}

void AGCageDoorActor::SynchronizeDoorCollision()
{
	if (!IsValid(DoorMesh) || !IsValid(DoorCollision) || !IsValid(DoorMesh->GetStaticMesh()))
	{
		return;
	}

	if (DoorCollision->GetAttachParent() != DoorMesh)
	{
		DoorCollision->AttachToComponent(DoorMesh, FAttachmentTransformRules::KeepRelativeTransform);
	}

	const FBoxSphereBounds DoorBounds = DoorMesh->GetStaticMesh()->GetBounds();
	const float DoorOuterX = DoorBounds.Origin.X + DoorBounds.BoxExtent.X;
	const float DoorEdgeY = DoorBounds.Origin.Y - DoorBounds.BoxExtent.Y;
	float FrameEdgeY = DoorEdgeY;
	if (IsValid(FrameMesh) && IsValid(FrameMesh->GetStaticMesh()))
	{
		const FBoxSphereBounds FrameBounds = FrameMesh->GetStaticMesh()->GetBounds();
		FrameEdgeY = FrameBounds.Origin.Y - FrameBounds.BoxExtent.Y;
	}
	DoorHinge->SetRelativeLocation(FVector(DoorOuterX, FrameEdgeY, 0.0f));
	DoorMesh->SetRelativeLocation(FVector(-DoorOuterX, -DoorEdgeY, 0.0f));
	DoorCollision->SetRelativeTransform(FTransform(FRotator::ZeroRotator, DoorBounds.Origin));
	DoorCollision->SetBoxExtent(DoorBounds.BoxExtent);
}

void AGCageDoorActor::OpenDoor()
{
	bTargetOpen = true;
}

void AGCageDoorActor::CloseDoor()
{
	bTargetOpen = false;
}

void AGCageDoorActor::ToggleDoor()
{
	bTargetOpen = !bTargetOpen;
}

void AGCageDoorActor::HandleDoorClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed)
{
	ToggleDoor();
}
