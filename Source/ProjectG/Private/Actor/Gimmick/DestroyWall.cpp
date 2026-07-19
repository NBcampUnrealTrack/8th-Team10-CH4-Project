#include "Actor/Gimmick/DestroyWall.h"
#include "Actor/GCageDoorActor.h"
#include "Components/StaticMeshComponent.h"
#include "Curves/CurveFloat.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Game/GProjectGameState.h"

ADestroyWall::ADestroyWall()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false; 
    
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;
    
    bIsSinking = false;
}

void ADestroyWall::BeginPlay()
{
    Super::BeginPlay();
    StartLocation = GetActorLocation();
    
    if (SinkCurve)
    {
        FOnTimelineFloat ProgressUpdateDelegate;
        ProgressUpdateDelegate.BindUFunction(this, FName("HandleTimelineProgress"));
        SinkTimeline.AddInterpFloat(SinkCurve, ProgressUpdateDelegate);

        FOnTimelineEvent FinishedDelegate;
        FinishedDelegate.BindUFunction(this, FName("HandleTimelineFinished"));
        SinkTimeline.SetTimelineFinishedFunc(FinishedDelegate);
    }

    AGProjectGameState* GS = Cast<AGProjectGameState>(UGameplayStatics::GetGameState(GetWorld()));
    if (GS)
    {
        GS->OnMatchTimeChanged.AddUObject(this, &ADestroyWall::OnMatchTimeUpdated);
    }
}

void ADestroyWall::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (SinkTimeline.IsPlaying())
    {
       SinkTimeline.TickTimeline(DeltaTime);
    }
}

void ADestroyWall::OnMatchTimeUpdated(int32 NewRemainTime)
{
    AGProjectGameState* GS = Cast<AGProjectGameState>(UGameplayStatics::GetGameState(GetWorld()));
    if (!GS) 
    {
        return;
    }
    
    int32 CurrentMaxRoundTime = GS->GetRoundDuration();
    
    if (CurrentMaxRoundTime <= 0)
    {
        return;
    }
    
    if (NewRemainTime >= CurrentMaxRoundTime && bIsSinking)
    {
        ResetWall();
        return;
    }
    
    if (bIsSinking) 
    {
        return;
    }
    
    if (NewRemainTime <= 0 || NewRemainTime >= CurrentMaxRoundTime) 
    {
        return; 
    }
    
    int32 TargetRemainTime = FMath::RoundToInt(CurrentMaxRoundTime * DestroyTimeRatio);
    
    if (NewRemainTime <= TargetRemainTime)
    {
        bIsSinking = true;
        SetActorTickEnabled(true);

        if (SinkCurve)
        {
            SinkTimeline.PlayFromStart();
            
            if (DestroySound)
            {
                UGameplayStatics::PlaySoundAtLocation(
                    this, 
                    DestroySound, 
                    GetActorLocation()
                );
            }
        }     
    }
}

void ADestroyWall::HandleTimelineProgress(float Value)
{
    float CurrentTime = SinkTimeline.GetPlaybackPosition();
    
    if (CurrentTime < 3.0f)
    {
        float ShakeX = FMath::FRandRange(-5.0f, 5.0f);
        float ShakeY = FMath::FRandRange(-5.0f, 5.0f);
        
        float ShakeZ = 0.0f; 

        FVector ShakeLocation = StartLocation + FVector(ShakeX, ShakeY, ShakeZ);
        SetActorLocation(ShakeLocation);
    }
    else
    {
		// The first three seconds only shake the intact floor. Starting physics
		// here prevents the cage from going to sleep before its support moves.
		// Every stage piece may call this; StartCageFall is idempotent.
		if (HasAuthority())
		{
			const FBox WallBounds = GetComponentsBoundingBox(true);
			for (TActorIterator<AGCageDoorActor> It(GetWorld()); It; ++It)
			{
				const FBox CageBounds = It->GetComponentsBoundingBox(true);
				const FVector CageLocation = It->GetActorLocation();
				const bool bInsideWallFootprint =
					CageLocation.X >= WallBounds.Min.X && CageLocation.X <= WallBounds.Max.X &&
					CageLocation.Y >= WallBounds.Min.Y && CageLocation.Y <= WallBounds.Max.Y;
				const float VerticalGap = CageBounds.Min.Z - WallBounds.Max.Z;
				const bool bStandingOnWall = VerticalGap >= -20.0f && VerticalGap <= 150.0f;

				if (bInsideWallFootprint && bStandingOnWall)
				{
					It->StartCageFall();
				}
			}
		}

        FVector TargetLocation = StartLocation - FVector(0.0f, 0.0f, 1500.0f);
        FVector CurrentLocation = FMath::Lerp(StartLocation, TargetLocation, Value);
        SetActorLocation(CurrentLocation);
    }
}

void ADestroyWall::HandleTimelineFinished()
{
    SetActorTickEnabled(false);
    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);
}

void ADestroyWall::ResetWall()
{
    bIsSinking = false;
    SinkTimeline.Stop();
    
    SetActorLocation(StartLocation);
    SetActorHiddenInGame(false);
    SetActorEnableCollision(true); 
    SetActorTickEnabled(false);
}
