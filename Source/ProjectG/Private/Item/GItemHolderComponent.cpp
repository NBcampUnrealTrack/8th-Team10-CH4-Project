#include "Item/GItemHolderComponent.h"
#include "Item/Consumable/GConsumableDefinition.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/Combo/GProjectComboData.h"
#include "GameplayEffect.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "Item/GItemPickup.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Character/GProjectCharacter.h"


UGItemHolderComponent::UGItemHolderComponent()
{
    PrimaryComponentTick.bCanEverTick = false;

    SetIsReplicatedByDefault(true);

    ConsumableMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ConsumableMesh"));
    ConsumableMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void UGItemHolderComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UGItemHolderComponent, ConsumableItem);
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

bool UGItemHolderComponent::HasNearbyPickup() const
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return false;
    }
    TArray<AActor*> Overlapping;
    Owner->GetOverlappingActors(Overlapping, AGItemPickup::StaticClass());
    return Overlapping.Num() > 0;
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

void UGItemHolderComponent::ClearHeldItem()
{
    if (GetOwnerRole() != ROLE_Authority)
    {
        return;
    }

    if (!ConsumableItem)
    {
        return;
    }

    ConsumableItem = nullptr;

    RefreshConsumableMesh();

    if (AActor* Owner = GetOwner())
    {
        Owner->ForceNetUpdate();
    }
}

void UGItemHolderComponent::BeginPlay()
{
    Super::BeginPlay();

    if (ACharacter* OwnerChar = Cast<ACharacter>(GetOwner()))
    {
        if (USkeletalMeshComponent* CharMesh = OwnerChar->GetMesh())
        {
            ConsumableMeshComponent->AttachToComponent(
                CharMesh,
                FAttachmentTransformRules::SnapToTargetNotIncludingScale,
                ConsumableSocketName);
        }
    }
    RefreshConsumableMesh();
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

    ConsumableItem = NewItem;
    RefreshConsumableMesh();
}

void UGItemHolderComponent::UseHeldItem()
{
    if (GetOwnerRole() != ROLE_Authority)
    {
        return;
    }

    if (!ConsumableItem)
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

    for (const TSubclassOf<UGameplayEffect>& EffectClass : ConsumableItem->EffectsToApply)
    {
        if (!EffectClass)
        {
            continue;
        }

        FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
        Context.AddSourceObject(this);

        FGameplayEffectSpecHandle Spec =
            ASC->MakeOutgoingSpec(EffectClass, ConsumableItem->EffectLevel, Context);

        if (Spec.IsValid())
        {
            ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
        }
    }

    if (ConsumableItem->UseEffect)
    {
        Multicast_PlayUseEffect(ConsumableItem->UseEffect);
    }

    ConsumableItem = nullptr;
    RefreshConsumableMesh();
}

void UGItemHolderComponent::OnRep_ConsumableItem()
{
    RefreshConsumableMesh();
}

void UGItemHolderComponent::RefreshConsumableMesh()
{
    if (!ConsumableMeshComponent)
    {
        return;
    }
    ConsumableMeshComponent->SetStaticMesh(ConsumableItem ? ConsumableItem->HeldMesh : nullptr);
}