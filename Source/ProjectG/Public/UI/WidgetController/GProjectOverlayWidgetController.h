// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/GProjectWidgetController.h"
#include "GProjectOverlayWidgetController.generated.h"

class AGProjectPlayerState;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FGProjectOnPlayerListChangedSignature);

UCLASS(BlueprintType, Blueprintable)
class PROJECTG_API UGProjectOverlayWidgetController : public UGProjectWidgetController
{
	GENERATED_BODY()

public:
	virtual void BindCallbacksToDependencies() override;

	TArray<AGProjectPlayerState*> GetOrderedPlayerStates() const;

	UPROPERTY(BlueprintAssignable, Category = "Player List")
	FGProjectOnPlayerListChangedSignature OnPlayerListChanged;

private:
	void HandlePlayerListChanged();
};
