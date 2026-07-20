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
class UGProjectChatWidget;

UCLASS()
class PROJECTG_API AGProjectPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AGProjectPlayerController();

	UFUNCTION(BlueprintCallable, Category = "Chat")
	void SendChatMessage(const FString& Message);

	void RegisterChatWidget(UGProjectChatWidget* InChatWidget);
	void UnRegisterChatWidget(UGProjectChatWidget* InChatWidget);

	void CloseChat();

	UFUNCTION(Server, Reliable)
	void ServerRequestReturnToLobby();

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> ChatAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Spectate")
	TObjectPtr<UInputAction> SpectatePrevAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Spectate")
	TObjectPtr<UInputAction> SpectateNextAction;

private:
	UGProjectAbilitySystemComponent* GetASC();
	bool IsGameplayInputBlocked();
	void InitHUD();

	void Move(const FInputActionValue& InputActionValue);
	void JumpPressed();
	void JumpReleased();
	
	void SpectatePrev();
	void SpectateNext();
	int32 CurrentSpectateIndex = 0;
	
	void AbilityInputTagPressed(FGameplayTag InputTag);
	void AbilityInputTagReleased(FGameplayTag InputTag);
	void AbilityInputTagHeld(FGameplayTag InputTag);
	void SendAttackInputEvent(FGameplayTag InputTag);
	void HandleAttackInputPressed(FGameplayTag InputTag);
	void FlushPendingAttackInput();
	void ClearPendingAttackInput();
	void TryStartParryInput();
	
	void OpenChat();
	bool bChatOpen = false;
	TWeakObjectPtr<UGProjectChatWidget> ChatWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Parry", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float ParryChordWindow = 0.08f;

	bool bBasicAttackHeld = false;
	bool bStrongAttackHeld = false;
	FGameplayTag PendingAttackInputTag;
	FTimerHandle PendingAttackInputTimer;

	UFUNCTION(Server, Reliable)
	void ServerSendAttackInputEvent(FGameplayTag InputTag);

	UFUNCTION(Server, Reliable)
	void ServerSendChatMessage(const FString& Message);

	UFUNCTION(Server, Reliable)
	void ServerSetPlayerName(const FString& InName);
	
	UFUNCTION(Server, Reliable)
	void ServerChangeSpectateTarget(int32 Direction);

	UPROPERTY()
	TObjectPtr<UGProjectAbilitySystemComponent> GProjectAbilitySystemComponent;
};
