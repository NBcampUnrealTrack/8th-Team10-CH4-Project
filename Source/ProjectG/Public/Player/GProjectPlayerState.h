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

DECLARE_MULTICAST_DELEGATE(FGProjectReadyChangedSignature)

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

	int32 GetPlayerColorIndex() const { return PlayerColorIndex; }

	void SetPlayerColorIndex(int32 NewColorIndex);

	FGProjectTeamChangedSignature OnTeamChanged;

	FString GetPlayerName() const { return PlayerName; }
	void SetPlayerName(const FString& InName) override;

	UPROPERTY(ReplicatedUsing = OnRep_IsReady)
	bool bIsReady = false;

	UPROPERTY(Replicated)
	bool bIsHost = false;

	UFUNCTION()
	void OnRep_IsReady();

	void SetReady(bool bNewReady);

	FGProjectReadyChangedSignature OnReadyChanged;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability System")
	TObjectPtr<UGProjectAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability System")
	TObjectPtr<UGProjectAttributeSet> AttributeSet;

	UFUNCTION()
	void OnRep_PlayerColorIndex();

private:
	UPROPERTY(ReplicatedUsing = OnRep_Team)
	EGProjectTeam Team = EGProjectTeam::None;

	UPROPERTY(ReplicatedUsing = OnRep_PlayerColorIndex, VisibleInstanceOnly, Category = "Player|Color")
	int32 PlayerColorIndex = INDEX_NONE;

	UFUNCTION()
	void OnRep_Team();

	UFUNCTION()
	virtual void CopyProperties(APlayerState* NewPlayerState) override;

	UPROPERTY(Replicated)
	FString PlayerName;
};
