#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TimelineComponent.h"
#include "DestroyWall.generated.h"

class UCurveFloat;

UCLASS()
class PROJECTG_API ADestroyWall : public AActor
{
	GENERATED_BODY()
	
public:	
	ADestroyWall();
	
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;
	
	bool bIsSinking;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Timeline", meta = (AllowPrivateAccess = "true"))
	UCurveFloat* SinkCurve;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RoundSettings", meta = (AllowPrivateAccess = "true"))
	int32 MaxRoundTime;

	FTimeline SinkTimeline;
	FVector StartLocation;
	
	void CheckRoundTime();
	
	FTimerHandle TimeCheckTimerHandle;
	
	UFUNCTION()
	void HandleTimelineProgress(float Value);
    
	UFUNCTION()
	void HandleTimelineFinished();
	
	UFUNCTION()
	void OnMatchTimeUpdated(int32 NewRemainTime);
};
