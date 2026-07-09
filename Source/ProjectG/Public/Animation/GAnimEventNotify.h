#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "GAnimEventNotify.generated.h"

UCLASS(meta = (DisplayName = "GProject Anim Event"))
class PROJECTG_API UGAnimEventNotify : public UAnimNotify
{
    GENERATED_BODY()

public:
    virtual void Notify(
        USkeletalMeshComponent* MeshComp,
        UAnimSequenceBase* Animation,
        const FAnimNotifyEventReference& EventReference) override;

protected:
    UPROPERTY(EditAnywhere, Category = "Event")
    FGameplayTag EventTag;
};