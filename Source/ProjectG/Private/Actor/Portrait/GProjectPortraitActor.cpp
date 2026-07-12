// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Portrait/GProjectPortraitActor.h"

#include "Character/GProjectCharacter.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "TimerManager.h"

#include "Player/GProjectPlayerColors.h"

AGProjectPortraitActor::AGProjectPortraitActor()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = false;
	SetActorEnableCollision(false);

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	PortraitMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PortraitMesh"));
	PortraitMesh->SetupAttachment(SceneRoot);
	PortraitMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PortraitMesh->SetGenerateOverlapEvents(false);

	PortraitCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("PortraitCapture"));
	PortraitCapture->SetupAttachment(SceneRoot);

	PortraitCapture->bCaptureEveryFrame = false;
	PortraitCapture->bCaptureOnMovement = false;

	PortraitCapture->FOVAngle = 30.0f;
	PortraitCapture->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;

	PortraitCapture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;

	PortraitCapture->SetRelativeLocation(FVector(180.0f, 0.0f, 155.0f));
	PortraitCapture->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));

	PortraitLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("PortraitLight"));
	PortraitLight->SetupAttachment(SceneRoot);
	PortraitLight->SetRelativeLocation(FVector(100.0f, -100.0f, 200.0f));
	PortraitLight->SetIntensity(4000.0f);
	PortraitLight->SetAttenuationRadius(500.0f);
	PortraitLight->SetCastShadows(false);
}

void AGProjectPortraitActor::InitializePortrait(
	AGProjectCharacter* SourceCharacter,
	const int32 PlayerColorIndex)
{
	if (!IsValid(SourceCharacter))
	{
		return;
	}

	USkeletalMeshComponent* SourceMesh = SourceCharacter->GetMesh();

	if (!SourceMesh || !SourceMesh->GetSkeletalMeshAsset())
	{
		return;
	}

	PortraitMesh->SetSkeletalMeshAsset(SourceMesh->GetSkeletalMeshAsset());

	const FLinearColor PlayerColor = GProjectPlayerColors::GetColor(PlayerColorIndex);

	PortraitMaterials.Reset();

	const int32 MaterialCount = SourceMesh->GetNumMaterials();

	for (int32 MaterialIndex = 0; MaterialIndex < MaterialCount; ++MaterialIndex)
	{
		UMaterialInterface* SourceMaterial = SourceMesh->GetMaterial(MaterialIndex);

		if (!SourceMaterial)
		{
			continue;
		}

		UMaterialInterface* ValidParentMaterial = SourceMaterial;

		while (UMaterialInstanceDynamic* SourceMID = Cast<UMaterialInstanceDynamic>(ValidParentMaterial))
		{
			ValidParentMaterial = SourceMID->Parent.Get();
			if (!ValidParentMaterial)
			{
				break;
			}
		}

		if (!ValidParentMaterial)
		{
			UE_LOG(
				LogTemp,
				Error,
				TEXT(
					"[Portrait] No valid material parent: Slot=%d"
				),
				MaterialIndex
			);

			continue;
		}

		UMaterialInstanceDynamic* PortraitMID = UMaterialInstanceDynamic::Create(ValidParentMaterial, this);
		if (!PortraitMID)
		{
			continue;
		}

		PortraitMID->SetVectorParameterValue(PlayerColorParameterName, PlayerColor);

		PortraitMID->SetScalarParameterValue(DissolveParameterName, 0.0f);

		PortraitMesh->SetMaterial(MaterialIndex, PortraitMID);

		PortraitMaterials.Add(PortraitMID);
	}

	RenderTarget =
		UKismetRenderingLibrary::CreateRenderTarget2D(
			this,
			RenderTargetSize,
			RenderTargetSize,
			ETextureRenderTargetFormat::RTF_RGBA8,
			FLinearColor(0.02f, 0.02f, 0.02f, 1.0f),
			false,
			false
		);

	if (!RenderTarget)
	{
		return;
	}

	PortraitCapture->TextureTarget = RenderTarget;

	PortraitCapture->ClearShowOnlyComponents();
	PortraitCapture->ShowOnlyActorComponents(this, true);

	GetWorldTimerManager().SetTimerForNextTick(this, &ThisClass::CapturePortrait);
}

void AGProjectPortraitActor::CapturePortrait()
{
	if (!PortraitCapture ||
		!PortraitCapture->TextureTarget)
	{
		return;
	}

	PortraitCapture->CaptureScene();
}