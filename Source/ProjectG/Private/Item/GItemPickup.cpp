#include "Item/GItemPickup.h"
#include "Item/GItemHolderComponent.h"
#include "Item/Consumable/GConsumableDefinition.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"

AGItemPickup::AGItemPickup()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    SetReplicateMovement(true);

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

    InitialTransform = GetActorTransform();

    OverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &AGItemPickup::OnOverlapBegin);
    OverlapSphere->OnComponentEndOverlap.AddDynamic(this, &AGItemPickup::OnOverlapEnd);

    ApplyPickupAvailable();
}

void AGItemPickup::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AGItemPickup, bPickupAvailable);
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

void AGItemPickup::SetPickupAvailable(const bool bAvailable)
{
    if (!HasAuthority())
    {
        return;
    }

    bPickupAvailable = bAvailable;

    ApplyPickupAvailable();

    ForceNetUpdate();
}

void AGItemPickup::ApplyPickupAvailable()
{
    SetActorHiddenInGame(!bPickupAvailable);

    if (OverlapSphere)
    {
        OverlapSphere->SetCollisionEnabled(
            bPickupAvailable
            ? ECollisionEnabled::QueryOnly
            : ECollisionEnabled::NoCollision
        );

        OverlapSphere->SetGenerateOverlapEvents(
            bPickupAvailable
        );

        OverlapSphere->UpdateOverlaps();
    }

    if (MeshComponent)
    {
        MeshComponent->SetVisibility(
            bPickupAvailable,
            true
        );

        MeshComponent->SetHiddenInGame(
            !bPickupAvailable
        );
    }

    if (!bPickupAvailable)
    {
        OverlappingActor = nullptr;
    }
}
void AGItemPickup::OnRep_PickupAvailable()
{
    ApplyPickupAvailable();
}

bool AGItemPickup::TryPickup(AActor* Picker)
{
    if (!HasAuthority())
    {
        return false;
    }

    if (!bPickupAvailable)
    {
        return false;
    }

    if (!Picker || !ItemDefinition || !OverlapSphere)
    {
        return false;
    }

    if (!OverlapSphere->IsOverlappingActor(Picker))
    {
        return false;
    }

    UGItemHolderComponent* Holder =
        Picker->FindComponentByClass<UGItemHolderComponent>();

    if (!Holder)
    {
        return false;
    }

    Holder->HoldItem(ItemDefinition);

    SetPickupAvailable(false);

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

    SetPickupAvailable(true);
}