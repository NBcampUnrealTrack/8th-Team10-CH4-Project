// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "GameplayTagContainer.h"
#include "GProjectAttackTraceNotifyState.generated.h"

UCLASS(meta = (DisplayName = "GProject Attack Hit Window"))
class PROJECTG_API UGProjectAttackTraceNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	FName GetTraceSocketName() const { return TraceSocketName; }

	virtual void NotifyBegin(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		float TotalDuration,
		const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyTick(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		float FrameDeltaTime,
		const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyEnd(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

private:
	void SendTraceEvent(USkeletalMeshComponent* MeshComp, const FGameplayTag& EventTag) const;

	UPROPERTY(EditAnywhere, Category = "Trace")
	FName TraceSocketName = TEXT("hand_r");
};
