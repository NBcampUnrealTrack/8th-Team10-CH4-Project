#include "Item/GItemHolderComponent.h"
#include "Item/Consumable/GConsumableDefinition.h"
#include "Item/GItemPickup.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffect.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"

UGItemHolderComponent::UGItemHolderComponent()
{
    PrimaryComponentTick.bCanEverTick = false;

    HeldMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeldMesh"));
    HeldMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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
}

void UGItemHolderComponent::HoldItem(UGConsumableDefinition* NewItem)
{
    if (!NewItem)
    {
        return;
    }

    HeldItem = NewItem;

    if (HeldMeshComponent)
    {
        HeldMeshComponent->SetStaticMesh(NewItem->HeldMesh);
    }
}

void UGItemHolderComponent::UseHeldItem()
{
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

    HeldItem = nullptr;

    if (HeldMeshComponent)
    {
        HeldMeshComponent->SetStaticMesh(nullptr);
    }
}