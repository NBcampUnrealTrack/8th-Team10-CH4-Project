#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GProjectTransform.generated.h"

class UAnimInstance;
class UGProjectComboData;
class USkeletalMesh;
class UAnimMontage;

UCLASS()
class PROJECTG_API UGProjectTransform : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Transform")
	TObjectPtr<USkeletalMesh> TransformMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Transform")
	TSubclassOf<UAnimInstance> TransformAnimClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Transform")
	FVector MeshRelativeLocation = FVector(0.0f, 0.0f, -90.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Transform")
	FRotator MeshRelativeRotation = FRotator(0.0f, -90.0f, 0.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Transform")
	float MeshScale = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Transform")
	float CapsuleRadius = 34.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Transform")
	float CapsuleHalfHeight = 90.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Transform|Movement")
	float WalkSpeed = 200.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Transform|Movement")
	float SprintSpeed = 450.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Transform|Death")
	TObjectPtr<UAnimMontage> DeathMontage;

	/*UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Transform|Death")
	float DeathDissolveDelay = 3.0f;*/

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Transform|Combat")
	TObjectPtr<UGProjectComboData> GroundComboData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Transform|Combat")
	TObjectPtr<UGProjectComboData> AirComboData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Transform|Combat")
	TObjectPtr<UGProjectComboData> DashComboData;
};