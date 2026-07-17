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
	void ResetWall();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USceneComponent* SceneRootComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UBoxComponent* CollisionBox;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* MeshComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Setting", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DestroyTimeRatio = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gimmick|Sound")
	class USoundBase* DestroySound;
	
private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Timeline", meta = (AllowPrivateAccess = "true"))
	UCurveFloat* SinkCurve;
	
	FTimeline SinkTimeline;
	FVector StartLocation;
	
	FTimerHandle TimeCheckTimerHandle;
	
	UFUNCTION()
	void HandleTimelineProgress(float Value);
    
	UFUNCTION()
	void HandleTimelineFinished();
	
	UFUNCTION()
	void OnMatchTimeUpdated(int32 NewRemainTime);
};
