#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ItemSpawnDataAsset.generated.h"

UCLASS()
class PROJECTG_API UItemSpawnDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FText ItemName;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	TSubclassOf<AActor> ItemClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", meta = (ClampMin = "0.0"))
	float SpawnWeight = 10.0f;
};
