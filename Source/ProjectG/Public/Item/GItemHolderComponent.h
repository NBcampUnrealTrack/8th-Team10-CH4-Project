#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GItemHolderComponent.generated.h"


class UGConsumableDefinition;
class UStaticMeshComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECTG_API UGItemHolderComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UGItemHolderComponent();

    UFUNCTION(BlueprintCallable, Category = "Item")
    void HoldItem(UGConsumableDefinition* NewItem);

    UFUNCTION(BlueprintCallable, Category = "Item")
    void UseHeldItem();

    UFUNCTION(BlueprintCallable, Category = "Item")
    bool HasHeldItem() const { return HeldItem != nullptr; }

    UFUNCTION(BlueprintCallable, Category = "Item")
    bool TryPickupNearby();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
    TObjectPtr<UGConsumableDefinition> HeldItem;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
    TObjectPtr<UStaticMeshComponent> HeldMeshComponent;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FName AttachSocketName = TEXT("hand_r");
};