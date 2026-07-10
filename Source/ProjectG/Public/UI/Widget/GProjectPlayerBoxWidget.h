// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/GProjectUserWidget.h"
#include "TImerManager.h"
#include "GProjectPlayerBoxWidget.generated.h"

class UImage;
class UProgressBar;
class UTextBlock;
class UTexture2D;
class UBorder;
class AGProjectCharacter;
class AGProjectPlayerState;
class AGProjectPortraitActor;

enum class EGProjectTeam : uint8;

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

	void ApplyTeamStyle(EGProjectTeam NewTeam);

	void SetupPortrait(AGProjectPlayerState* InPlayerState);

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeWidgetControllerSet() override;
	virtual void NativeDestruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> NameText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> CharacterImage;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> HPBar;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> HPBar_Yellow;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> SPBar;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> HPText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SPText;

	UPROPERTY(meta = (BIndWidget))
	TObjectPtr<UBorder> PlayerFrame;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> PortraitImage;

	UPROPERTY(EditDefaultsOnly, Category = "Portrait")
	TSubclassOf<AGProjectPortraitActor> PortraitActorClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "PlayerBox|HP Delay")
	float HPDelayTime = 0.5f; 

	UPROPERTY(EditDefaultsOnly, Category = "PlayerBox|HP Delay")
	float HPInterpSpeed = 3.0f; 

private:
	void RefreshHealth();
	void RefreshSP();
	void TrySetupPortrait();

	float Health = 0.0f;
	float MaxHealth = 1.0f;
	float SP = 0.0f;
	float MaxSP = 1.0f;
	
	float HPDelayTimer = 0.0f;

	TWeakObjectPtr<AGProjectPlayerState>
		PortraitPlayerState;

	FTimerHandle PortraitRetryTimerHandle;

	UPROPERTY(Transient)
	TObjectPtr<AGProjectPortraitActor> PortraitActor;
};
