#include "Actor/Gimmick/DestroyWall.h"
#include "Components/StaticMeshComponent.h"
#include "Curves/CurveFloat.h"
#include "Kismet/GameplayStatics.h"
#include "Game/GProjectGameState.h"

ADestroyWall::ADestroyWall()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false; 
    
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;
    
    MaxRoundTime = 30;
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
    if (NewRemainTime >= MaxRoundTime && bIsSinking)
    {
        ResetWall();
        return;
    }

    if (bIsSinking) return;

    if (NewRemainTime <= 0 || NewRemainTime >= MaxRoundTime) 
    {
        return; 
    }
    
    int32 TargetRemainTime = FMath::RoundToInt(MaxRoundTime * DestroyTimeRatio);

    if (NewRemainTime <= TargetRemainTime)
    {
        bIsSinking = true;
        SetActorTickEnabled(true);
        
        if (SinkCurve)
        {
            SinkTimeline.PlayFromStart();
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