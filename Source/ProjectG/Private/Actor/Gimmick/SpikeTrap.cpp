// Copyright Epic Games, Inc. All Rights Reserved.

#include "Actor/Gimmick/SpikeTrap.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/GProjectAbilitySystemLibrary.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Curves/CurveFloat.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

ASpikeTrap::ASpikeTrap()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	SceneRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRootComponent"));
	RootComponent = SceneRootComponent;

	BaseMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMeshComponent"));
	BaseMeshComponent->SetupAttachment(RootComponent);

	SpikeMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpikeMeshComponent"));
	SpikeMeshComponent->SetupAttachment(RootComponent);
	SpikeMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(RootComponent);
	TriggerBox->SetCollisionProfileName(TEXT("Trigger"));

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
}

void ASpikeTrap::BeginPlay()
{
	Super::BeginPlay();

	SpikeRetractedLocation = SpikeMeshComponent->GetRelativeLocation();
	SpikeExtendedLocation = SpikeRetractedLocation + FVector(0.0f, 0.0f, RiseHeight);

	if (SpikeCurve)
	{
		FOnTimelineFloat ProgressUpdateDelegate;
		ProgressUpdateDelegate.BindUFunction(this, FName("HandleTimelineProgress"));
		SpikeTimeline.AddInterpFloat(SpikeCurve, ProgressUpdateDelegate);

		FOnTimelineEvent FinishedDelegate;
		FinishedDelegate.BindUFunction(this, FName("HandleTimelineFinished"));
		SpikeTimeline.SetTimelineFinishedFunc(FinishedDelegate);
	}

	if (HasAuthority())
	{
		TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ASpikeTrap::OnTriggerBoxBeginOverlap);
	}
}

void ASpikeTrap::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (SpikeTimeline.IsPlaying())
	{
		SpikeTimeline.TickTimeline(DeltaTime);
	}
}

void ASpikeTrap::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASpikeTrap, bSpikeExtended);
}

void ASpikeTrap::OnTriggerBoxBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!HasAuthority() || bOnCooldown || bSpikeExtended || WarningTimerHandle.IsValid())
	{
		return;
	}

	if (!Cast<ACharacter>(OtherActor))
	{
		return;
	}

	GetWorldTimerManager().SetTimer(WarningTimerHandle, this, &ASpikeTrap::ServerExtendSpike, WarningDelay, false);
}

void ASpikeTrap::ServerExtendSpike()
{
	WarningTimerHandle.Invalidate();
	bOnCooldown = true;
	bSpikeExtended = true;
	PlayExtendTimeline();
}

void ASpikeTrap::ServerRetractSpike()
{
	bSpikeExtended = false;
	PlayRetractTimeline();
}

void ASpikeTrap::OnRep_SpikeExtended()
{
	if (bSpikeExtended)
	{
		PlayExtendTimeline();
	}
	else
	{
		PlayRetractTimeline();
	}
}

void ASpikeTrap::PlayExtendTimeline()
{
	if (SpikeCurve)
	{
		SpikeTimeline.Play();
	}

	if (SpikeSound)
	{
		GetWorldTimerManager().SetTimer(SoundTimerHandle, this, &ASpikeTrap::PlaySpikeSound, SpikeSoundDelay, false);
	}
}

void ASpikeTrap::PlayRetractTimeline()
{
	if (SpikeCurve)
	{
		SpikeTimeline.Reverse();
	}
}

void ASpikeTrap::HandleTimelineProgress(float Value)
{
	SpikeMeshComponent->SetRelativeLocation(FMath::Lerp(SpikeRetractedLocation, SpikeExtendedLocation, Value));
}

void ASpikeTrap::HandleTimelineFinished()
{
	if (!HasAuthority())
	{
		return;
	}

	if (bSpikeExtended)
	{
		ApplyDamageToOverlappingTargets();
		GetWorldTimerManager().SetTimer(RetractTimerHandle, this, &ASpikeTrap::ServerRetractSpike, ExtendedHoldTime, false);
	}
	else
	{
		GetWorldTimerManager().SetTimer(CooldownTimerHandle, this, &ASpikeTrap::ClearCooldown, CooldownTime, false);
	}
}

void ASpikeTrap::ApplyDamageToOverlappingTargets()
{
	if (!HasAuthority() || !SpikeDamageEffectClass)
	{
		return;
	}

	TArray<AActor*> OverlappingActors;
	TriggerBox->GetOverlappingActors(OverlappingActors, ACharacter::StaticClass());

	for (AActor* Target : OverlappingActors)
	{
		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
		if (!TargetASC)
		{
			continue;
		}

		FGProjectDamageEffectParams DamageParams;
		DamageParams.SourceAbilitySystemComponent = AbilitySystemComponent;
		DamageParams.TargetAbilitySystemComponent = TargetASC;
		DamageParams.BaseDamage = DamageAmount;

		UGProjectAbilitySystemLibrary::ApplyDamageEffect(DamageParams, SpikeDamageEffectClass);
	}
}

void ASpikeTrap::ClearCooldown()
{
	bOnCooldown = false;
}

// 지연된 스파이크 사운드 재생
void ASpikeTrap::PlaySpikeSound()
{
	UGameplayStatics::PlaySoundAtLocation(this, SpikeSound, SpikeMeshComponent->GetComponentLocation(), SpikeSoundVolume);
}
