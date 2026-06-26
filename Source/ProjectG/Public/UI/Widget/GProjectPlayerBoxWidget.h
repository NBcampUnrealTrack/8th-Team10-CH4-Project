// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/GProjectUserWidget.h"
#include "GProjectPlayerBoxWidget.generated.h"

class UImage;
class UProgressBar;
class UTextBlock;
class UTexture2D;

UCLASS()
class PROJECTG_API UGProjectPlayerBoxWidget : public UGProjectUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "PlayerBox")
	void SetPlayerName(const FText& NewName);

	UFUNCTION(BlueprintCallable, Category = "PlayerBox")
	void SetCharacterImage(UTexture2D* NewImage);

	UFUNCTION(BlueprintCallable, Category = "PlayerBox")
	void SetHealth(float NewHealth);

	UFUNCTION(BlueprintCallable, Category = "PlayerBox")
	void SetMaxHealth(float NewMaxHealth);

	UFUNCTION(BlueprintCallable, Category = "PlayerBox")
	void SetSP(float NewSP);

	UFUNCTION(BlueprintCallable, Category = "PlayerBox")
	void SetMaxSP(float NewMaxSP);

protected:
	virtual void NativePreConstruct() override;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> NameText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> CharacterImage;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> HPBar;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> SPBar;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> HPText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SPText;

private:
	void RefreshHealth();
	void RefreshSP();

	float Health = 0.0f;
	float MaxHealth = 1.0f;
	float SP = 0.0f;
	float MaxSP = 1.0f;
};
