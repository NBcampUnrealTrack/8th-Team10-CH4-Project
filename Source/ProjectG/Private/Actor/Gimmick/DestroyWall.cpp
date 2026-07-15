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
    if (bIsSinking) return;

    int32 HalfTime = MaxRoundTime / 2;

    if (NewRemainTime > 0 && NewRemainTime <= HalfTime)
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
    FVector TargetLocation = StartLocation - FVector(0.0f, 0.0f, 1500.0f);
    FVector CurrentLocation = FMath::Lerp(StartLocation, TargetLocation, Value);
    SetActorLocation(CurrentLocation);
}

void ADestroyWall::HandleTimelineFinished()
{
    Destroy();
}