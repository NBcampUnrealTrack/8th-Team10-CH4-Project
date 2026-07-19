// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GProjectTitlePlayerController.generated.h"

class UGProjectTitleWidget;

UCLASS()
class PROJECTG_API AGProjectTitlePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AGProjectTitlePlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Title UI")
	TSubclassOf<UGProjectTitleWidget> TitleWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UGProjectTitleWidget> TitleWidget;
};
