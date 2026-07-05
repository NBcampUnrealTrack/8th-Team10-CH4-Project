#include "Item/GItemHolderComponent.h"
#include "Item/Consumable/GConsumableDefinition.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffect.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "Item/GItemPickup.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"


UGItemHolderComponent::UGItemHolderComponent()
{
    PrimaryComponentTick.bCanEverTick = false;

    SetIsReplicatedByDefault(true);

    HeldMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeldMesh"));
    HeldMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void UGItemHolderComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UGItemHolderComponent, HeldItem);
}

bool UGItemHolderComponent::TryPickupNearby()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return false;
    }

    TArray<AActor*> Overlapping;
    Owner->GetOverlappingActors(Overlapping, AGItemPickup::StaticClass());

    for (AActor* Actor : Overlapping)
    {
        if (AGItemPickup* Pickup = Cast<AGItemPickup>(Actor))
        {
            if (Pickup->TryPickup(Owner))
            {
                return true;
            }
        }
    }

    return false;
}

void UGItemHolderComponent::Multicast_PlayUseEffect_Implementation(UNiagaraSystem* Effect)
{
    if (!Effect)
    {
        return;
    }

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        Owner->GetWorld(),
        Effect,
        Owner->GetActorLocation() + FVector(0,0,50.f),
        Owner->GetActorRotation());
}

void UGItemHolderComponent::BeginPlay()
{
    Super::BeginPlay();

    if (ACharacter* OwnerChar = Cast<ACharacter>(GetOwner()))
    {
        if (USkeletalMeshComponent* CharMesh = OwnerChar->GetMesh())
        {
            HeldMeshComponent->AttachToComponent(
                CharMesh,
                FAttachmentTransformRules::SnapToTargetNotIncludingScale,
                AttachSocketName);
        }
    }

    RefreshHeldMesh();
}

void UGItemHolderComponent::HoldItem(UGConsumableDefinition* NewItem)
{
    if (GetOwnerRole() != ROLE_Authority)
    {
        return;
    }

    if (!NewItem)
    {
        return;
    }

    HeldItem = NewItem;

    RefreshHeldMesh();
}

void UGItemHolderComponent::UseHeldItem()
{
    if (GetOwnerRole() != ROLE_Authority)
    {
        return;
    }

    if (!HeldItem)
    {
        return;
    }

    AActor* Owner = GetOwner();
    UAbilitySystemComponent* ASC =
        UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner);

    if (!ASC)
    {
        return;
    }

    for (const TSubclassOf<UGameplayEffect>& EffectClass : HeldItem->EffectsToApply)
    {
        if (!EffectClass)
        {
            continue;
        }

        FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
        Context.AddSourceObject(this);

        FGameplayEffectSpecHandle Spec =
            ASC->MakeOutgoingSpec(EffectClass, HeldItem->EffectLevel, Context);

        if (Spec.IsValid())
        {
            ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
        }
    }

    if (HeldItem->UseEffect)
    {
        Multicast_PlayUseEffect(HeldItem->UseEffect);
    }

    HeldItem = nullptr;

    RefreshHeldMesh();
}

void UGItemHolderComponent::OnRep_HeldItem()
{
    RefreshHeldMesh();
}

void UGItemHolderComponent::RefreshHeldMesh()
{
    if (!HeldMeshComponent)
    {
        return;
    }

    HeldMeshComponent->SetStaticMesh(HeldItem ? HeldItem->HeldMesh : nullptr);
}