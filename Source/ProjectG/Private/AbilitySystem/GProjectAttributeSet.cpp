// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/GProjectAttributeSet.h"

#include "Character/GProjectCharacter.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"
#include "Game/GProjectGameState.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Player/GProjectPlayerState.h"

namespace
{
	AGProjectPlayerState* ResolvePlayerState(
		AActor* Actor)
	{
		if (!IsValid(Actor))
		{
			return nullptr;
		}

		if (AGProjectPlayerState* PlayerState = Cast<AGProjectPlayerState>(Actor))
		{
			return PlayerState;
		}

		if (APawn* Pawn = Cast<APawn>(Actor))
		{
			return Pawn ->GetPlayerState<AGProjectPlayerState>();
		}

		if (AController* Controller = Cast<AController>(Actor))
		{
			return Controller->GetPlayerState<AGProjectPlayerState>();
		}

		if (APawn* InstigatorPawn = Actor->GetInstigator())
		{
			return InstigatorPawn ->GetPlayerState<AGProjectPlayerState>();
		}

		return nullptr;
	}
}

UGProjectAttributeSet::UGProjectAttributeSet()
{
	InitIncomingDamage(0.0f);
	InitHealth(100.0f);
	InitMaxHealth(100.0f);
	InitSP(100.0f);
	InitMaxSP(100.0f);
	InitMoveSpeed(600.0f);
	InitAttackPower(10.0f);
	InitDefense(0.0f);
	InitKnockbackPower(1.0f);
	InitKnockbackResistance(0.0f);
}

void UGProjectAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UGProjectAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGProjectAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGProjectAttributeSet, SP, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGProjectAttributeSet, MaxSP, COND_None, REPNOTIFY_Always);
}

void UGProjectAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	ClampAttribute(Attribute, NewValue);
}

void UGProjectAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		const float Damage = GetIncomingDamage();
		SetIncomingDamage(0.0f);

		if (Damage <= 0.0f)
		{
			return;
		}

		const float OldHealth = GetHealth();
		const float NewHealth = FMath::Clamp(OldHealth - Damage, 0.0f, GetMaxHealth());
		SetHealth(NewHealth);

		if (OldHealth > 0.0f && NewHealth <= 0.0f)
		{
			AGProjectCharacter* TargetCharacter = Cast<AGProjectCharacter>(Data.Target.GetAvatarActor());
			if (!TargetCharacter)
			{
				return;
			}

			AGProjectPlayerState* VictimPlayerState = TargetCharacter->GetPlayerState<AGProjectPlayerState>();

			const FGameplayEffectContextHandle Context = Data.EffectSpec.GetContext();

			AGProjectPlayerState* KillerPlayerState = ResolvePlayerState(Context.GetOriginalInstigator());
			if (!KillerPlayerState)
			{
				KillerPlayerState = ResolvePlayerState(Context.GetEffectCauser());
			}

			if (UWorld* World = TargetCharacter->GetWorld())
			{
				if (AGProjectGameState* GameState = World->GetGameState<AGProjectGameState>())
				{
					GameState->BroadcastKillFeed(
						KillerPlayerState,
						VictimPlayerState
					);
				}
			}

			TargetCharacter->HandleDeath();
		}
	}
	else if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
	}
	else if (Data.EvaluatedData.Attribute == GetSPAttribute())
	{
		SetSP(FMath::Clamp(GetSP(), 0.0f, GetMaxSP()));
	}
}

void UGProjectAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGProjectAttributeSet, Health, OldHealth);
}

void UGProjectAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGProjectAttributeSet, MaxHealth, OldMaxHealth);
}

void UGProjectAttributeSet::OnRep_SP(const FGameplayAttributeData& OldSP) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGProjectAttributeSet, SP, OldSP);
}

void UGProjectAttributeSet::OnRep_MaxSP(const FGameplayAttributeData& OldMaxSP) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGProjectAttributeSet, MaxSP, OldMaxSP);
}

void UGProjectAttributeSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	else if (Attribute == GetMaxHealthAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f);
	}
	else if (Attribute == GetSPAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxSP());
	}
	else if (Attribute == GetMaxSPAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
	else if (Attribute == GetMoveSpeedAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
}

#undef ATTRIBUTE_ACCESSORS
