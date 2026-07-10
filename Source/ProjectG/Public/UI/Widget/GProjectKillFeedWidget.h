#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GProjectKillFeedWidget.generated.h"

class UVerticalBox;
class UGProjectKillFeedEntryWidget;

UCLASS()
class PROJECTG_API UGProjectKillFeedWidget
	: public UUserWidget
{
	GENERATED_BODY()

public:
	void AddKillFeedEntry(
		const FString& KillerName,
		int32 KillerColorIndex,
		const FString& VictimName,
		int32 VictimColorIndex
	);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> KillFeedContainer;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Kill Feed"
	)
	TSubclassOf<UGProjectKillFeedEntryWidget>
		KillFeedEntryWidgetClass;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Kill Feed",
		meta = (ClampMin = "1")
	)
	int32 MaxVisibleEntries = 5;
};