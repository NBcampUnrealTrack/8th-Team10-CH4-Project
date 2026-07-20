#pragma once

#include "CoreMinimal.h"

namespace GProjectPlayerColors
{
	inline FLinearColor GetColor(const int ColorIndex)
	{
		static const FLinearColor Colors[] =
		{
			FLinearColor(1.0f, 0.0f, 0.0f),
			FLinearColor(0.0f, 1.0f, 0.0f),
			FLinearColor(0.0f, 0.0f, 1.0f),
			FLinearColor(1.0f, 1.0f, 1.0f)
		};

		if (ColorIndex < 0 || ColorIndex >= UE_ARRAY_COUNT(Colors))
		{
			return FLinearColor::White;
		}
		return Colors[ColorIndex];
	}
}
