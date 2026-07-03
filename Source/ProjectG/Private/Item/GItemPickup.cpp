#include "Item/GItemPickup.h"
#include "Item/GItemHolderComponent.h"
#include "Item/Consumable/GConsumableDefinition.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"

AGItemPickup::AGItemPickup()
{
    PrimaryActorTick.bCanEverTick = false;

    OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));
    OverlapSphere->InitSphereRadius(100.0f);
    OverlapSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    RootComponent = OverlapSphere;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    MeshComponent->SetupAttachment(RootComponent);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AGItemPickup::BeginPlay()
{
    Super::BeginPlay();

    OverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &AGItemPickup::OnOverlapBegin);
    OverlapSphere->OnComponentEndOverlap.AddDynamic(this, &AGItemPickup::OnOverlapEnd);
}

void AGItemPickup::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor)
    {
        return;
    }

    if (OtherActor->FindComponentByClass<UGItemHolderComponent>())
    {
        OverlappingActor = OtherActor;
    }
}

void AGItemPickup::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor == OverlappingActor)
    {
        OverlappingActor = nullptr;
    }
}

bool AGItemPickup::TryPickup(AActor* Picker)
{
    if (!Picker || Picker != OverlappingActor || !ItemDefinition)
    {
        return false;
    }

    UGItemHolderComponent* Holder = Picker->FindComponentByClass<UGItemHolderComponent>();
    if (!Holder)
    {
        return false;
    }

    Holder->HoldItem(ItemDefinition);
    Destroy();
    return true;
}