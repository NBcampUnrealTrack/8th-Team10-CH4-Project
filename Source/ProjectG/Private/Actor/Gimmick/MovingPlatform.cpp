// Copyright Epic Games, Inc. All Rights Reserved.

#include "Actor/Gimmick/MovingPlatform.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"

AMovingPlatform::AMovingPlatform()
{
	PrimaryActorTick.bCanEverTick = true;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;
}

void AMovingPlatform::BeginPlay()
{
	Super::BeginPlay();

	StartLocation = GetActorLocation();
	NormalizedDirection = MoveDirection.GetSafeNormal();
}

void AMovingPlatform::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MoveDistance <= 0.0f || MoveSpeed <= 0.0f || NormalizedDirection.IsNearlyZero())
	{
		return;
	}

	// 왕복 이동 전체를 서버 동기화 시간만의 함수로 계산해서, 클라이언트마다 리플리케이션 없이도
	// 서로 동일한 위치를 얻도록 함 (DestroyWall이 GameState 시간을 공유하는 방식과 같은 취지).
	// GetWorld()->GetTimeSeconds()는 각자 로컬 월드 기준 시간이라 접속 시점에 따라 어긋나므로,
	// 서버-클라이언트가 공유하는 GameState의 GetServerWorldTimeSeconds()를 사용해야 함
	AGameStateBase* GameState = GetWorld()->GetGameState();
	if (!GameState)
	{
		return;
	}

	// 한 방향 이동 + 도착 후 대기(HoldTime)를 왕복 양쪽에 넣은 전체 주기
	const float TravelTime = MoveDistance / MoveSpeed;
	const float Period = TravelTime * 2.0f + HoldTime * 2.0f;
	const float ElapsedTime = FMath::Fmod(GameState->GetServerWorldTimeSeconds(), Period);

	float TraveledDistance;
	if (ElapsedTime < TravelTime)
	{
		// 정방향 이동 구간
		TraveledDistance = MoveSpeed * ElapsedTime;
	}
	else if (ElapsedTime < TravelTime + HoldTime)
	{
		// 도착 지점에서 대기하는 구간
		TraveledDistance = MoveDistance;
	}
	else if (ElapsedTime < TravelTime * 2.0f + HoldTime)
	{
		// 역방향 이동 구간
		TraveledDistance = MoveDistance - MoveSpeed * (ElapsedTime - TravelTime - HoldTime);
	}
	else
	{
		// 시작 지점에서 대기하는 구간
		TraveledDistance = 0.0f;
	}

	SetActorLocation(StartLocation + NormalizedDirection * TraveledDistance);
}
