// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "GProjectPlayerState.generated.h"

class UAbilitySystemComponent;
class UGProjectAbilitySystemComponent;
class UGProjectAttributeSet;

UENUM(BlueprintType)
enum class EGProjectTeam : uint8
{
	None UMETA(DisplayName = "None"),
	Red UMETA(DisplayName = "Red Team"),
	Blue UMETA(DisplayName = "Blue Team")
};

DECLARE_MULTICAST_DELEGATE_OneParam(
	FGProjectTeamChangedSignature,
	EGProjectTeam
)

UCLASS()
class PROJECTG_API AGProjectPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AGProjectPlayerState();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UGProjectAbilitySystemComponent* GetGProjectAbilitySystemComponent() const;
	UGProjectAttributeSet* GetAttributeSet() const;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SetTeam(EGProjectTeam NewTeam);

	UFUNCTION(BlueprintPure, Category = "Team")
	EGProjectTeam GetTeam() const { return Team; }

	FGProjectTeamChangedSignature OnTeamChanged;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability System")
	TObjectPtr<UGProjectAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability System")
	TObjectPtr<UGProjectAttributeSet> AttributeSet;

private:
	UPROPERTY(ReplicatedUsing = OnRep_Team)
	EGProjectTeam Team = EGProjectTeam::None;

	UFUNCTION()
	void OnRep_Team();
};
