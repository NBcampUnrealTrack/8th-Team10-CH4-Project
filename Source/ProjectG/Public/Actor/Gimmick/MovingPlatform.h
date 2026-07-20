// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MovingPlatform.generated.h"

UCLASS()
class PROJECTG_API AMovingPlatform : public AActor
{
	GENERATED_BODY()

public:
	AMovingPlatform();

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Movement")
	FVector MoveDirection = FVector(1.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Movement", meta = (ClampMin = "0.0"))
	float MoveDistance = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Movement", meta = (ClampMin = "1.0"))
	float MoveSpeed = 100.0f;

	// 양 끝 지점에 도착했을 때 방향을 바꾸기 전 멈춰 있는 시간(초)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Movement", meta = (ClampMin = "0.0"))
	float HoldTime = 1.0f;

private:
	FVector StartLocation;
	FVector NormalizedDirection;
};
