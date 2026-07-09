// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GProjectPortraitActor.generated.h"

class USceneComponent;
class USkeletalMeshComponent;
class USceneCaptureComponent2D;
class UPointLightComponent;
class UTextureRenderTarget2D;
class UMaterialInstanceDynamic;
class AGProjectCharacter;

UCLASS()
class PROJECTG_API AGProjectPortraitActor : public AActor
{
	GENERATED_BODY()

public:
	AGProjectPortraitActor();

	void InitializePortrait(
		AGProjectCharacter* SourceCharacter,
		int32 PlayerColorIndex
	);

	UTextureRenderTarget2D* GetRenderTarget() const
	{
		return RenderTarget;
	}

	void CapturePortrait();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portrait")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portrait")
	TObjectPtr<USkeletalMeshComponent> PortraitMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portrait")
	TObjectPtr<USceneCaptureComponent2D> PortraitCapture;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portrait")
	TObjectPtr<UPointLightComponent> PortraitLight;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Portrait|RenderTarget",
		meta = (ClampMin = "64")
	)
	int32 RenderTargetSize = 256;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Portrait")
	FName PlayerColorParameterName = TEXT("PlayerColor");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Portrait")
	FName DissolveParameterName = TEXT("DissolveAmount");

private:
	UPROPERTY(Transient)
	TObjectPtr<UTextureRenderTarget2D> RenderTarget;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInstanceDynamic>> PortraitMaterials;
};