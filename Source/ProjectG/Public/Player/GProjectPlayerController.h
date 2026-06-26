// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "GProjectPlayerController.generated.h"

struct FInputActionValue;
class UInputAction;
class UInputMappingContext;
class UGProjectInputConfig;
class UGProjectAbilitySystemComponent;

UCLASS()
class PROJECTG_API AGProjectPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AGProjectPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnRep_PlayerState() override;
	virtual void SetupInputComponent() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UGProjectInputConfig> InputConfig;

private:
	UGProjectAbilitySystemComponent* GetASC();
	void InitHUD();

	void Move(const FInputActionValue& InputActionValue);
	void JumpPressed();
	void JumpReleased();

	void AbilityInputTagPressed(FGameplayTag InputTag);
	void AbilityInputTagReleased(FGameplayTag InputTag);
	void AbilityInputTagHeld(FGameplayTag InputTag);

	UPROPERTY()
	TObjectPtr<UGProjectAbilitySystemComponent> GProjectAbilitySystemComponent;
};
