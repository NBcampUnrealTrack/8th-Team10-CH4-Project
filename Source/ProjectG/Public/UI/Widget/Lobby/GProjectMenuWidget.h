// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GProjectMenuWidget.generated.h"

class UButton;
class UScrollBox;
class UEditableText;
class UComboBoxString;
class UBorder;
class UTextBlock;
class UGProjectSessionRowWidget;
class UGProjectProfileSettingsWidget;
struct FBattleMapData;

UCLASS()
class PROJECTG_API UGProjectMenuWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UGProjectMenuWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnHostButtonClicked();

	UFUNCTION()
	void OnJoinButtonClicked();
	
	UFUNCTION()
	void OnExitButtonClicked();
	
	UFUNCTION()
	void OnFindSessionsCompleteUpdateUI(
		const TArray<FString>& SessionNames,
		const TArray<FString>& MapNames,
		bool bWasSuccessful
	);

	UFUNCTION()
	void HandleSessionRowClicked(int32 SessionIndex);

	UFUNCTION()
	void OnConfirmNoResultsClicked();

	UFUNCTION()
	void OnConfirmHostButtonClicked();

	UFUNCTION()
	void OnPrevMapClicked();
	UFUNCTION()
	void OnNextMapClicked();

	UFUNCTION()
	void OnPrevPlayersClicked();
	UFUNCTION()
	void OnNextPlayersClicked();

	UFUNCTION()
	void OnProfileSettingsButtonClicked();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
	TObjectPtr<USoundBase> CommonClickSound;
	UFUNCTION()
	void OnAnyButtonClicked();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI", Meta = (AllowPrivateAccess, BindWidget))
	TObjectPtr<UBorder> HostSettingsBorder;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI", Meta = (AllowPrivateAccess, BindWidget))
	TObjectPtr<UEditableText> RoomNameInput;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Map Data", Meta = (AllowPrivateAccess))
	TArray<FBattleMapData> AvailableBattleMaps;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI", Meta = (AllowPrivateAccess, BindWidget))
	TObjectPtr<UButton> ConfirmHostButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "USTitleWidget", Meta = (AllowPrivateAccess, BindWidget))
	TObjectPtr<UButton> HostButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "USTitleWidget", Meta = (AllowPrivateAccess, BindWidget))
	TObjectPtr<UButton> JoinButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "USTitleWidget", Meta = (AllowPrivateAccess, BindWidget))
	TObjectPtr<UButton> ProfileSettingsButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "USTitleWidget", Meta = (AllowPrivateAccess, BindWidget))
	TObjectPtr<UButton> ExitButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI", Meta = (AllowPrivateAccess, BindWidget))
	TObjectPtr<UScrollBox> SessionListScrollBox;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UGProjectSessionRowWidget> SessionRowWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UUserWidget> ProfileWidgetClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI", Meta = (AllowPrivateAccess, BindWidget))
	TObjectPtr<UBorder> NoResultsBorder;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI", Meta = (AllowPrivateAccess, BindWidget))
	TObjectPtr<UButton> ConfirmNoResultsButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BtnPrevMap;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BtnNextMap;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextMapName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BtnPrevPlayers;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BtnNextPlayers;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextMaxPlayers;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Map Data", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDataTable> MapDataTable;

	TArray<FBattleMapData> CachedBattleMaps;

	int32 CurrentMapIndex = 0;

	int32 CurrentMaxPlayers = 2;

	void InitMapDataFromTable();

	void UpdateMapSelectionUI();

	void UpdatePlayerSelectionUI();
};
