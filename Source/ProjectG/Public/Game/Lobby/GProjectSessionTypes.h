#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GProjectSessionTypes.generated.h"

USTRUCT(BlueprintType)
struct FBattleMapData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Data")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Data")
	FString MapPath;
};

USTRUCT(BlueprintType)
struct FPlayerProfileData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerName = TEXT("DefaultPlayer");
};