#include "Item/SpawnItem/SpawnBase.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

ASpawnBase::ASpawnBase()
{
	PrimaryActorTick.bCanEverTick = false;
	
	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	SetRootComponent(Scene);
    
	SpawningBox = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawningBox"));
	SpawningBox->SetupAttachment(Scene);
}

FVector ASpawnBase::GetRandomPointInVolume() const
{
	FVector BoxExtent = SpawningBox->GetScaledBoxExtent();
	FVector BoxOrigin = SpawningBox->GetComponentLocation();
	
	return BoxOrigin + FVector(
	   FMath::FRandRange(-BoxExtent.X, BoxExtent.X),
	   FMath::FRandRange(-BoxExtent.Y, BoxExtent.Y),
	   -BoxExtent.Z + 10.0f
	);
}

void ASpawnBase::SpawnItem(TSubclassOf<AActor> ItemClass)
{
	if (!ItemClass) return;

	GetWorld()->SpawnActor<AActor>(
	   ItemClass,
	   GetRandomPointInVolume(),
	   FRotator::ZeroRotator
	);
}
