// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "GProjectPlayerState.generated.h"

class UAbilitySystemComponent;
class UGProjectAbilitySystemComponent;
class UGProjectAttributeSet;

UCLASS()
class PROJECTG_API AGProjectPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AGProjectPlayerState();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UGProjectAbilitySystemComponent* GetGProjectAbilitySystemComponent() const;
	UGProjectAttributeSet* GetAttributeSet() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability System")
	TObjectPtr<UGProjectAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability System")
	TObjectPtr<UGProjectAttributeSet> AttributeSet;
};
