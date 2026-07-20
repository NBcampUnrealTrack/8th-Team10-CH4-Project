// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "GProjectHUD.generated.h"

class APlayerController;
class APlayerState;
class UAbilitySystemComponent;
class UAttributeSet;
class UGProjectOverlayWidget;
class UGProjectOverlayWidgetController;

UCLASS()
class PROJECTG_API AGProjectHUD : public AHUD
{
	GENERATED_BODY()

public:
	void InitOverlay(APlayerController* InPlayerController, APlayerState* InPlayerState, UAbilitySystemComponent* InASC, UAttributeSet* InAttributeSet);

	UFUNCTION(BlueprintPure, Category = "UI")
	UGProjectOverlayWidgetController* GetOverlayWidgetController() const { return OverlayWidgetController; }

protected:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UGProjectOverlayWidget> OverlayWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UGProjectOverlayWidgetController> OverlayWidgetControllerClass;

private:
	UPROPERTY()
	TObjectPtr<UGProjectOverlayWidget> OverlayWidget;

	UPROPERTY()
	TObjectPtr<UGProjectOverlayWidgetController> OverlayWidgetController;

};
