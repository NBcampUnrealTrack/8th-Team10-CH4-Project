// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GProjectAttackHitNotify.generated.h"

UCLASS(meta = (DisplayName = "GProject Attack Hit"))
class PROJECTG_API UGProjectAttackHitNotify : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
};
