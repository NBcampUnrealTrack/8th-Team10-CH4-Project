// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/Widget/GProjectTitleWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TimerManager.h"

namespace
{
	UTextBlock* MakeText(UWidgetTree* WidgetTree, const TCHAR* Name, const FString& Text, int32 Size, const FLinearColor& Color)
	{
		UTextBlock* TextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
		TextBlock->SetText(FText::FromString(Text));
		TextBlock->SetColorAndOpacity(FSlateColor(Color));
		TextBlock->SetJustification(ETextJustify::Center);
		FSlateFontInfo Font = TextBlock->GetFont();
		Font.Size = Size;
		Font.TypefaceFontName = TEXT("Bold");
		TextBlock->SetFont(Font);
		return TextBlock;
	}

	void FillCanvas(UCanvasPanelSlot* Slot, int32 ZOrder = 0)
	{
		Slot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
		Slot->SetOffsets(FMargin(0.0f));
		Slot->SetZOrder(ZOrder);
	}
}

UGProjectTitleWidget::UGProjectTitleWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsFocusable(true);
	BackgroundShadeColor = FLinearColor(0.0f, 0.0f, 0.0f, 0.34f);
	TitleText = FText::GetEmpty();
	SubtitleText = FText::GetEmpty();
	AccentLineColor = FLinearColor(0.48f, 0.025f, 0.015f, 0.32f);
	StartButtonText = FText::FromString(TEXT("PRESS ANY KEY"));
	StartButtonColor = FLinearColor(0.16f, 0.025f, 0.018f, 0.92f);
	ExitButtonText = FText::FromString(TEXT("EXIT"));
	ExitButtonColor = FLinearColor(0.04f, 0.04f, 0.05f, 0.85f);
}

void UGProjectTitleWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (!WidgetTree || WidgetTree->RootWidget)
	{
		return;
	}

	UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("TitleRoot"));
	WidgetTree->RootWidget = Root;

	UBorder* Background = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Background"));
	Background->SetBrushColor(FLinearColor(0.005f, 0.008f, 0.018f, 1.0f));
	FillCanvas(Root->AddChildToCanvas(Background));

	if (BackgroundTexture)
	{
		UImage* BackgroundImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("BackgroundImage"));
		BackgroundImage->SetBrushFromTexture(BackgroundTexture, true);
		BackgroundImage->SetBrushTintColor(FSlateColor(FLinearColor::White));
		FillCanvas(Root->AddChildToCanvas(BackgroundImage), 1);
	}

	UBorder* BackgroundShade = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("BackgroundShade"));
	BackgroundShade->SetBrushColor(BackgroundShadeColor);
	FillCanvas(Root->AddChildToCanvas(BackgroundShade), 2);

	if (bShowAccentLine)
	{
		UBorder* Accent = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Accent"));
		Accent->SetBrushColor(AccentLineColor);
		UCanvasPanelSlot* AccentSlot = Root->AddChildToCanvas(Accent);
		AccentSlot->SetZOrder(3);
		AccentSlot->SetAnchors(FAnchors(0.0f, 0.42f, 1.0f, 0.58f));
		AccentSlot->SetOffsets(FMargin(0.0f));
	}

	if (!TitleText.IsEmpty())
	{
		UTextBlock* Title = MakeText(WidgetTree, TEXT("GameTitle"), TitleText.ToString(), 84, FLinearColor(0.96f, 0.82f, 0.58f, 1.0f));
		UCanvasPanelSlot* TitleSlot = Root->AddChildToCanvas(Title);
		TitleSlot->SetZOrder(4);
		TitleSlot->SetAnchors(FAnchors(0.5f, 0.40f));
		TitleSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		TitleSlot->SetSize(FVector2D(900.0f, 130.0f));
		TitleSlot->SetPosition(FVector2D::ZeroVector);
	}

	if (!SubtitleText.IsEmpty())
	{
		UTextBlock* Subtitle = MakeText(WidgetTree, TEXT("Subtitle"), SubtitleText.ToString(), 22, FLinearColor(0.78f, 0.70f, 0.60f, 0.85f));
		UCanvasPanelSlot* SubtitleSlot = Root->AddChildToCanvas(Subtitle);
		SubtitleSlot->SetZOrder(4);
		SubtitleSlot->SetAnchors(FAnchors(0.5f, 0.49f));
		SubtitleSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		SubtitleSlot->SetSize(FVector2D(600.0f, 45.0f));
	}

	StartPromptText = MakeText(WidgetTree, TEXT("StartPromptText"), StartButtonText.ToString(), 24, FLinearColor::White);
	StartPromptText->SetVisibility(ESlateVisibility::HitTestInvisible);
	UCanvasPanelSlot* StartSlot = Root->AddChildToCanvas(StartPromptText);
	StartSlot->SetZOrder(4);
	StartSlot->SetAnchors(FAnchors(0.5f, StartButtonVerticalPosition));
	StartSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	StartSlot->SetSize(StartButtonSize);

	ExitButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("ExitButton"));
	ExitButton->SetBackgroundColor(ExitButtonColor);
	ExitButton->AddChild(MakeText(WidgetTree, TEXT("ExitText"), ExitButtonText.ToString(), 16, FLinearColor(0.75f, 0.75f, 0.75f, 1.0f)));
	UCanvasPanelSlot* ExitSlot = Root->AddChildToCanvas(ExitButton);
	ExitSlot->SetZOrder(4);
	ExitSlot->SetAnchors(FAnchors(0.5f, ExitButtonVerticalPosition));
	ExitSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	ExitSlot->SetSize(ExitButtonSize);
}

void UGProjectTitleWidget::NativeConstruct()
{
	Super::NativeConstruct();

	bTransitioning = false;
	ElapsedSeconds = 0.0f;
	SetRenderOpacity(0.0f);

	if (ExitButton)
	{
		ExitButton->OnClicked.RemoveDynamic(this, &ThisClass::HandleExitClicked);
		ExitButton->OnClicked.AddDynamic(this, &ThisClass::HandleExitClicked);
	}
}

void UGProjectTitleWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(MenuTravelTimer);
	}
	if (ExitButton)
	{
		ExitButton->OnClicked.RemoveDynamic(this, &ThisClass::HandleExitClicked);
	}

	Super::NativeDestruct();
}

void UGProjectTitleWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	ElapsedSeconds += InDeltaTime;

	if (bTransitioning)
	{
		SetRenderOpacity(FMath::Max(0.0f, GetRenderOpacity() - InDeltaTime * 2.5f));
	}
	else
	{
		SetRenderOpacity(FMath::Min(1.0f, ElapsedSeconds * 1.5f));
		if (StartPromptText)
		{
			const float PromptOpacity = (FMath::Sin(ElapsedSeconds * 3.0f) + 1.0f) * 0.5f;
			StartPromptText->SetRenderOpacity(PromptOpacity);
		}
	}
}

FReply UGProjectTitleWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() != EKeys::Escape)
	{
		RequestStart();
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

FReply UGProjectTitleWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	RequestStart();
	return FReply::Handled();
}

void UGProjectTitleWidget::HandleExitClicked()
{
	if (!bTransitioning)
	{
		UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
	}
}

void UGProjectTitleWidget::RequestStart()
{
	if (bTransitioning)
	{
		return;
	}

	bTransitioning = true;
	SetIsEnabled(false);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(MenuTravelTimer, this, &ThisClass::OpenMainMenu, 0.4f, false);
	}
}

void UGProjectTitleWidget::OpenMainMenu()
{
	RemoveFromParent();
	UGameplayStatics::OpenLevel(this, TEXT("/Game/Level/MenuMap"));
}
