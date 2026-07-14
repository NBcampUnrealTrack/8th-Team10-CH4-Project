// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GProjectMenuWidget.generated.h"

class UButton;
class UScrollBox;
class UEditableText;
class UBorder;
class UGProjectSessionRowWidget;

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
	void OnFindSessionsCompleteUpdateUI(const TArray<FString>& SessionNames, bool bWasSuccessful);

	UFUNCTION()
	void HandleSessionRowClicked(int32 SessionIndex);

	UFUNCTION()
	void OnConfirmNoResultsClicked();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "USTitleWidget", Meta = (AllowPrivateAccess, BindWidget))
	TObjectPtr<UButton> HostButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "USTitleWidget", Meta = (AllowPrivateAccess, BindWidget))
	TObjectPtr<UButton> JoinButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "USTitleWidget", Meta = (AllowPrivateAccess, BindWidget))
	TObjectPtr<UButton> ExitButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI", Meta = (AllowPrivateAccess, BindWidget))
	TObjectPtr<UScrollBox> SessionListScrollBox;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UGProjectSessionRowWidget> SessionRowWidgetClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI", Meta = (AllowPrivateAccess, BindWidget))
	TObjectPtr<UBorder> NoResultsBorder;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI", Meta = (AllowPrivateAccess, BindWidget))
	TObjectPtr<UButton> ConfirmNoResultsButton;
};
