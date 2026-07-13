#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item/ItemSpawnDataAsset.h"
#include "SpawnBase.generated.h"


class UBoxComponent;

UCLASS()
class PROJECTG_API ASpawnBase : public AActor
{
	GENERATED_BODY()
	
public:	
	ASpawnBase();
	
	AActor* SpawnRandomItemFromZone(UItemSpawnDataAsset* ItemData);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Spawning")
	USceneComponent* Scene;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Spawning")
	UBoxComponent* SpawningBox;
	
	UFUNCTION(BlueprintCallable, Category="Spawning")
	FVector GetRandomPointInVolume() const;

	UFUNCTION(BlueprintCallable, Category="Spawning")
	void SpawnItem(TSubclassOf<AActor> ItemClass);
};
