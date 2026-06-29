// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GProjectUserWidget.generated.h"

UCLASS()
class PROJECTG_API UGProjectUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetWidgetController(UObject* InWidgetController);

	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<UObject> WidgetController;

protected:
	virtual void NativeWidgetControllerSet();

	UFUNCTION(BlueprintImplementableEvent, Category = "WidgetController")
	void WidgetControllerSet();
};
