#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GCageDoorActor.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UBoxComponent;

UCLASS(Blueprintable)
class PROJECTG_API AGCageDoorActor : public AActor
{
	GENERATED_BODY()

public:
	AGCageDoorActor();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual bool ShouldTickIfViewportsOnly() const override { return true; }

	UFUNCTION(BlueprintCallable, Category = "Cage Door")
	void OpenDoor();

	UFUNCTION(BlueprintCallable, Category = "Cage Door")
	void CloseDoor();

	UFUNCTION(BlueprintCallable, Category = "Cage Door")
	void ToggleDoor();

	UFUNCTION(BlueprintPure, Category = "Cage Door")
	bool IsDoorOpen() const { return bTargetOpen; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cage Door")
	TObjectPtr<UStaticMeshComponent> FrameMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cage Door")
	TObjectPtr<USceneComponent> DoorHinge;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cage Door")
	TObjectPtr<UStaticMeshComponent> DoorMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cage Door|Collision")
	TObjectPtr<UBoxComponent> BackWallCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cage Door|Collision")
	TObjectPtr<UBoxComponent> LeftWallCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cage Door|Collision")
	TObjectPtr<UBoxComponent> RightWallCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cage Door|Collision")
	TObjectPtr<UBoxComponent> FloorCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cage Door|Collision")
	TObjectPtr<UBoxComponent> CeilingCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cage Door|Collision")
	TObjectPtr<UBoxComponent> DoorCollision;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cage Door", meta = (ClampMin = "-170.0", ClampMax = "170.0"))
	float OpenAngle = -105.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cage Door", meta = (ClampMin = "1.0"))
	float OpenSpeed = 140.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cage Door")
	bool bStartOpen = false;

private:
	void SynchronizeDoorCollision();

	UFUNCTION()
	void HandleDoorClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed);

	bool bTargetOpen = false;
};
