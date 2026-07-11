#include "Item/GItemPickup.h"
#include "Item/GItemHolderComponent.h"
#include "Item/Consumable/GConsumableDefinition.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"

AGItemPickup::AGItemPickup()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

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

    SetPickupEnabled(true);
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
    if (!HasAuthority())
    {
        return false;
    }

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
    
    OverlappingActor = nullptr;

    SetPickupEnabled(false);

    ForceNetUpdate();

    return true;
}

void AGItemPickup::SetPickupEnabled(const bool bEnabled)
{
    SetActorHiddenInGame(!bEnabled);

    if (OverlapSphere)
    {
        OverlapSphere->SetCollisionEnabled(
            bEnabled
            ? ECollisionEnabled::QueryOnly
            : ECollisionEnabled::NoCollision
        );

        OverlapSphere->SetGenerateOverlapEvents(bEnabled);
    }

    if (MeshComponent)
    {
        MeshComponent->SetVisibility(bEnabled, true);
        MeshComponent->SetHiddenInGame(!bEnabled);
    }
}

void AGItemPickup::ResetForNewRound()
{
    if (!HasAuthority())
    {
        return;
    }

    SetActorTransform(
        InitialTransform,
        false,
        nullptr,
        ETeleportType::TeleportPhysics
    );

    OverlappingActor = nullptr;

    SetPickupEnabled(true);

    ForceNetUpdate();
}