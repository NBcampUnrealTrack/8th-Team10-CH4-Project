#include "UI/Widget/GProjectKillFeedWidget.h"

#include "Components/VerticalBox.h"
#include "UI/Widget/GProjectKillFeedEntryWidget.h"

void UGProjectKillFeedWidget::AddKillFeedEntry(
	const FString& KillerName,
	const int32 KillerColorIndex,
	const FString& VictimName,
	const int32 VictimColorIndex)
{
	if (!KillFeedContainer ||
		!KillFeedEntryWidgetClass)
	{
		return;
	}

\
	while (
		KillFeedContainer->GetChildrenCount() >=
		MaxVisibleEntries
		)
	{
		KillFeedContainer->RemoveChildAt(0);
	}

	UGProjectKillFeedEntryWidget* NewEntry =
		CreateWidget<UGProjectKillFeedEntryWidget>(
			GetOwningPlayer(),
			KillFeedEntryWidgetClass
		);

	if (!NewEntry)
	{
		return;
	}

	KillFeedContainer->AddChildToVerticalBox(
		NewEntry
	);

	NewEntry->InitializeKillFeedEntry(
		KillerName,
		KillerColorIndex,
		VictimName,
		VictimColorIndex
	);
}
