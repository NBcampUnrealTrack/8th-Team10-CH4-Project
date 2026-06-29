// Copyright Epic Games, Inc. All Rights Reserved.

#include "Actor/GProjectAttributeTrap.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameplayEffect.h"

AGProjectAttributeTrap::AGProjectAttributeTrap()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	TrapMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TrapMesh"));
	SetRootComponent(TrapMesh);
	TrapMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(TrapMesh);
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetGenerateOverlapEvents(true);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void AGProjectAttributeTrap::BeginPlay()
{
	Super::BeginPlay();

	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnTrapOverlap);
}

void AGProjectAttributeTrap::OnTrapOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!HasAuthority() || !OtherActor || !TrapEffectClass)
	{
		return;
	}

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
	if (!TargetASC)
	{
		return;
	}

	FGameplayEffectContextHandle EffectContext = TargetASC->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	const FGameplayEffectSpecHandle EffectSpec = TargetASC->MakeOutgoingSpec(TrapEffectClass, 1.0f, EffectContext);
	if (EffectSpec.IsValid())
	{
		TargetASC->ApplyGameplayEffectSpecToSelf(*EffectSpec.Data.Get());
	}
}
