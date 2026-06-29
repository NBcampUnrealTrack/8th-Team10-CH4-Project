// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GProjectAttributeTrap.generated.h"

class UBoxComponent;
class UGameplayEffect;
class UPrimitiveComponent;
class UStaticMeshComponent;

UCLASS()
class PROJECTG_API AGProjectAttributeTrap : public AActor
{
	GENERATED_BODY()

public:
	AGProjectAttributeTrap();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnTrapOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

private:
	UPROPERTY(VisibleAnywhere, Category = "Trap")
	TObjectPtr<UStaticMeshComponent> TrapMesh;

	UPROPERTY(VisibleAnywhere, Category = "Trap")
	TObjectPtr<UBoxComponent> TriggerBox;

	UPROPERTY(EditAnywhere, Category = "Trap")
	TSubclassOf<UGameplayEffect> TrapEffectClass;
};
