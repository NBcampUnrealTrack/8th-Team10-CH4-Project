// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Player/GProjectPlayerState.h"
#include "AGProjectCageActor.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class USceneComponent;

UCLASS()
class PROJECTG_API AGProjectCageActor : public AActor
{
	GENERATED_BODY()

public:
	AGProjectCageActor();

	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "Cage")
	void OpenDoor();

	UFUNCTION(BlueprintCallable, Category = "Cage")
	void CloseDoor();

	UFUNCTION(BlueprintCallable, Category = "Cage")
	void ToggleDoor();

	UFUNCTION(BlueprintPure, Category = "Cage")
	EGProjectTeam GetCageTeam() const { return CageTeam; }

	UFUNCTION(BlueprintPure, Category = "Cage")
	bool ShouldOpenWhenTeamMemberDies() const { return bOpenWhenTeamMemberDies; }

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere, Category = "Cage|Components")
	TObjectPtr<UBoxComponent> CagePhysicsRoot;

	UPROPERTY(VisibleAnywhere, Category = "Cage|Components")
	TObjectPtr<UStaticMeshComponent> CageFrameMesh;

	UPROPERTY(VisibleAnywhere, Category = "Cage|Components")
	TObjectPtr<USceneComponent> DoorPivot;

	UPROPERTY(VisibleAnywhere, Category = "Cage|Components")
	TObjectPtr<UStaticMeshComponent> CageDoorMesh;

	UPROPERTY(VisibleAnywhere, Category = "Cage|Components")
	TObjectPtr<UBoxComponent> BackWallCollision;

	UPROPERTY(VisibleAnywhere, Category = "Cage|Components")
	TObjectPtr<UBoxComponent> LeftWallCollision;

	UPROPERTY(VisibleAnywhere, Category = "Cage|Components")
	TObjectPtr<UBoxComponent> RightWallCollision;

	UPROPERTY(VisibleAnywhere, Category = "Cage|Components")
	TObjectPtr<UBoxComponent> FloorCollision;

	UPROPERTY(VisibleAnywhere, Category = "Cage|Components")
	TObjectPtr<UBoxComponent> CeilingCollision;

	UPROPERTY(VisibleAnywhere, Category = "Cage|Components")
	TObjectPtr<UBoxComponent> DoorCollision;

	UPROPERTY(EditAnywhere, Category = "Cage|Team")
	EGProjectTeam CageTeam = EGProjectTeam::None;

	UPROPERTY(EditAnywhere, Category = "Cage|Open")
	bool bOpenWhenTeamMemberDies = true;

	UPROPERTY(EditAnywhere, Category = "Cage|Open")
	bool bOpenByTimer = true;

	UPROPERTY(EditAnywhere, Category = "Cage|Open", meta = (EditCondition = "bOpenByTimer", ClampMin = "0.0"))
	float AutoOpenDelay = 30.0f;

	UPROPERTY(EditAnywhere, Category = "Cage|Open")
	float OpenRoll = -120.0f;

	UPROPERTY(EditAnywhere, Category = "Cage|Open", meta = (ClampMin = "0.1"))
	float DoorInterpSpeed = 5.0f;

	UPROPERTY(EditAnywhere, Category = "Cage|Collapse")
	bool bDisableGameplayCollisionsOnCollapse = true;

	UPROPERTY(EditAnywhere, Category = "Cage|Collapse")
	float CollapseImpulseStrength = 0.0f;

	UPROPERTY(ReplicatedUsing = OnRep_IsOpen)
	bool bIsOpen = false;

	UPROPERTY(EditAnywhere, Category = "Cage|Physics")
	bool bSimulatePhysicsFromStart = true;

	FRotator ClosedDoorPivotRotation = FRotator::ZeroRotator;

	FTimerHandle AutoOpenTimerHandle;

	UFUNCTION()
	void OnRep_IsOpen();

	void HandleAutoOpenTimer();

	void ApplyDoorCollisionState();

	float GetTargetDoorRoll() const;
};