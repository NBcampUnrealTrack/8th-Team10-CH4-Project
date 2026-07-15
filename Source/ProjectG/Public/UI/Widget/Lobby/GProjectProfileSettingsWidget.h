// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/GProjectUserWidget.h"
#include "GProjectProfileSettingsWidget.generated.h"

class UEditableText;
class UButton;

UCLASS()
class PROJECTG_API UGProjectProfileSettingsWidget : public UGProjectUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnSaveProfileClicked();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableText> PlayerNameInput;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BtnSaveProfile;
};