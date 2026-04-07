#include "Phase0/UI/BSPhase0HUDWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/GameInstance.h"
#include "Engine/Texture2D.h"
#include "Phase0/Actors/BSPhase0Character.h"
#include "Phase0/Components/BSInventoryComponent.h"
#include "Phase0/Components/BSSurvivorStateComponent.h"
#include "Phase0/Components/BSTaskComponent.h"
#include "Phase0/Data/BSItemDefinition.h"
#include "Phase0/Data/BSTaskDefinition.h"
#include "Phase0/Systems/BSSettlementSubsystem.h"

namespace
{
	template <typename TWidget>
	TWidget* MakeWidget(UWidgetTree* WidgetTree, const FName Name)
	{
		return WidgetTree ? WidgetTree->ConstructWidget<TWidget>(TWidget::StaticClass(), Name) : nullptr;
	}

	void SetTextFontSize(UTextBlock* TextBlock, const int32 FontSize)
	{
		if (!TextBlock)
		{
			return;
		}

		FSlateFontInfo FontInfo = TextBlock->GetFont();
		FontInfo.Size = FontSize;
		TextBlock->SetFont(FontInfo);
	}

	UTextBlock* AddText(UWidgetTree* WidgetTree, UVerticalBox* Parent, const FName Name, const FText& Text, const int32 FontSize, const FLinearColor& Color)
	{
		UTextBlock* TextBlock = MakeWidget<UTextBlock>(WidgetTree, Name);
		if (!TextBlock || !Parent)
		{
			return nullptr;
		}

		TextBlock->SetText(Text);
		TextBlock->SetColorAndOpacity(FSlateColor(Color));
		TextBlock->SetAutoWrapText(true);
		SetTextFontSize(TextBlock, FontSize);

		if (UVerticalBoxSlot* VerticalSlot = Parent->AddChildToVerticalBox(TextBlock))
		{
			VerticalSlot->SetPadding(FMargin(0.0f, 2.0f));
		}

		return TextBlock;
	}

	UProgressBar* AddBar(UWidgetTree* WidgetTree, UVerticalBox* Parent, const FName Name, const FLinearColor& FillColor)
	{
		UProgressBar* ProgressBar = MakeWidget<UProgressBar>(WidgetTree, Name);
		if (!ProgressBar || !Parent)
		{
			return nullptr;
		}

		ProgressBar->SetPercent(1.0f);
		ProgressBar->SetFillColorAndOpacity(FillColor);
		if (UVerticalBoxSlot* VerticalSlot = Parent->AddChildToVerticalBox(ProgressBar))
		{
			VerticalSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 6.0f));
		}

		return ProgressBar;
	}

	UBorder* MakePanel(UWidgetTree* WidgetTree, const FName Name, const FLinearColor& Color)
	{
		UBorder* Border = MakeWidget<UBorder>(WidgetTree, Name);
		if (Border)
		{
			Border->SetBrushColor(Color);
			Border->SetPadding(FMargin(14.0f));
		}

		return Border;
	}

	int32 CountTotalItems(const TArray<FBSItemStack>& Items)
	{
		int32 TotalCount = 0;
		for (const FBSItemStack& Stack : Items)
		{
			TotalCount += FMath::Max(0, Stack.Quantity);
		}

		return TotalCount;
	}
}

void UBSPhase0HUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
	BuildHud();
	RefreshNow();
}

void UBSPhase0HUDWidget::NativeTick(const FGeometry& MyGeometry, const float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	HudRefreshAccumulator += InDeltaTime;
	if (HudRefreshAccumulator >= 0.1f)
	{
		HudRefreshAccumulator = 0.0f;
		RefreshHud();
	}

	if (NotificationSecondsRemaining > 0.0f)
	{
		NotificationSecondsRemaining = FMath::Max(0.0f, NotificationSecondsRemaining - InDeltaTime);
		if (NotificationText)
		{
			NotificationText->SetText(ActiveNotification);
		}

		if (NotificationBorder)
		{
			const float Alpha = FMath::Clamp(NotificationSecondsRemaining / 0.35f, 0.0f, 1.0f);
			const FLinearColor BorderColor = ActiveNotificationColor.CopyWithNewOpacity(0.16f + (0.28f * Alpha));
			NotificationBorder->SetBrushColor(BorderColor);
			NotificationBorder->SetVisibility(NotificationSecondsRemaining > 0.0f ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		}
	}
	else if (NotificationBorder)
	{
		NotificationBorder->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UBSPhase0HUDWidget::SetBackpackVisible(const bool bVisible)
{
	bBackpackVisible = bVisible;

	if (BackpackPanel)
	{
		BackpackPanel->SetVisibility(bBackpackVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	RefreshInventoryPanel();
}

bool UBSPhase0HUDWidget::IsBackpackVisible() const
{
	return bBackpackVisible;
}

void UBSPhase0HUDWidget::PushNotification(const FText& Message, const FLinearColor& Color, const float DurationSeconds)
{
	ActiveNotification = Message;
	ActiveNotificationColor = Color;
	NotificationSecondsRemaining = FMath::Max(DurationSeconds, 0.1f);

	if (NotificationText)
	{
		NotificationText->SetText(ActiveNotification);
		NotificationText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	}

	if (NotificationBorder)
	{
		NotificationBorder->SetBrushColor(Color.CopyWithNewOpacity(0.32f));
		NotificationBorder->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}

void UBSPhase0HUDWidget::RefreshNow()
{
	HudRefreshAccumulator = 0.0f;
	LastInventorySignature.Reset();
	RefreshHud();
}

void UBSPhase0HUDWidget::BuildHud()
{
	if (WidgetTree->RootWidget)
	{
		return;
	}

	RootCanvas = MakeWidget<UCanvasPanel>(WidgetTree, TEXT("RootCanvas"));
	WidgetTree->RootWidget = RootCanvas;

	StressOverlay = MakePanel(WidgetTree, TEXT("StressOverlay"), FLinearColor(0.2f, 0.01f, 0.01f, 0.0f));
	if (UCanvasPanelSlot* CanvasSlot = RootCanvas->AddChildToCanvas(StressOverlay))
	{
		CanvasSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
		CanvasSlot->SetOffsets(FMargin(0.0f));
	}
	StressOverlay->SetVisibility(ESlateVisibility::HitTestInvisible);

	SettlementPanel = MakePanel(WidgetTree, TEXT("SettlementPanel"), FLinearColor(0.02f, 0.06f, 0.08f, 0.82f));
	if (UCanvasPanelSlot* CanvasSlot = RootCanvas->AddChildToCanvas(SettlementPanel))
	{
		CanvasSlot->SetAnchors(FAnchors(0.0f, 0.0f));
		CanvasSlot->SetPosition(FVector2D(22.0f, 22.0f));
		CanvasSlot->SetSize(FVector2D(340.0f, 160.0f));
	}
	UVerticalBox* SettlementBox = MakeWidget<UVerticalBox>(WidgetTree, TEXT("SettlementBox"));
	SettlementPanel->SetContent(SettlementBox);
	AddText(WidgetTree, SettlementBox, TEXT("SettlementHeaderText"), NSLOCTEXT("BlackStatic", "SettlementHeader", "Settlement Status"), 19, FLinearColor(0.90f, 0.95f, 1.0f));
	SettlementSummaryText = AddText(WidgetTree, SettlementBox, TEXT("SettlementSummaryText"), FText::GetEmpty(), 14, FLinearColor(0.80f, 0.88f, 0.93f));
	SettlementStatusText = AddText(WidgetTree, SettlementBox, TEXT("SettlementStatusText"), FText::GetEmpty(), 13, FLinearColor(0.63f, 0.80f, 0.85f));

	ObjectivePanel = MakePanel(WidgetTree, TEXT("ObjectivePanel"), FLinearColor(0.08f, 0.05f, 0.02f, 0.84f));
	if (UCanvasPanelSlot* CanvasSlot = RootCanvas->AddChildToCanvas(ObjectivePanel))
	{
		CanvasSlot->SetAnchors(FAnchors(0.5f, 0.0f));
		CanvasSlot->SetAlignment(FVector2D(0.5f, 0.0f));
		CanvasSlot->SetPosition(FVector2D(0.0f, 22.0f));
		CanvasSlot->SetSize(FVector2D(470.0f, 180.0f));
	}
	UVerticalBox* ObjectiveBox = MakeWidget<UVerticalBox>(WidgetTree, TEXT("ObjectiveBox"));
	ObjectivePanel->SetContent(ObjectiveBox);
	ObjectiveTitleText = AddText(WidgetTree, ObjectiveBox, TEXT("ObjectiveTitleText"), FText::GetEmpty(), 20, FLinearColor(0.98f, 0.88f, 0.70f));
	ObjectiveStepText = AddText(WidgetTree, ObjectiveBox, TEXT("ObjectiveStepText"), FText::GetEmpty(), 15, FLinearColor(1.0f, 0.97f, 0.92f));
	ObjectiveHintText = AddText(WidgetTree, ObjectiveBox, TEXT("ObjectiveHintText"), FText::GetEmpty(), 13, FLinearColor(0.82f, 0.78f, 0.72f));

	NotificationBorder = MakePanel(WidgetTree, TEXT("NotificationBorder"), FLinearColor(0.12f, 0.12f, 0.12f, 0.0f));
	if (UCanvasPanelSlot* CanvasSlot = RootCanvas->AddChildToCanvas(NotificationBorder))
	{
		CanvasSlot->SetAnchors(FAnchors(1.0f, 0.0f));
		CanvasSlot->SetAlignment(FVector2D(1.0f, 0.0f));
		CanvasSlot->SetPosition(FVector2D(-22.0f, 22.0f));
		CanvasSlot->SetSize(FVector2D(420.0f, 92.0f));
	}
	NotificationText = MakeWidget<UTextBlock>(WidgetTree, TEXT("NotificationText"));
	SetTextFontSize(NotificationText, 16);
	NotificationText->SetAutoWrapText(true);
	NotificationText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	NotificationBorder->SetContent(NotificationText);
	NotificationBorder->SetVisibility(ESlateVisibility::Collapsed);

	VitalsPanel = MakePanel(WidgetTree, TEXT("VitalsPanel"), FLinearColor(0.02f, 0.02f, 0.02f, 0.84f));
	if (UCanvasPanelSlot* CanvasSlot = RootCanvas->AddChildToCanvas(VitalsPanel))
	{
		CanvasSlot->SetAnchors(FAnchors(0.0f, 1.0f));
		CanvasSlot->SetAlignment(FVector2D(0.0f, 1.0f));
		CanvasSlot->SetPosition(FVector2D(22.0f, -22.0f));
		CanvasSlot->SetSize(FVector2D(360.0f, 230.0f));
	}
	UVerticalBox* VitalsBox = MakeWidget<UVerticalBox>(WidgetTree, TEXT("VitalsBox"));
	VitalsPanel->SetContent(VitalsBox);
	AddText(WidgetTree, VitalsBox, TEXT("VitalsHeaderText"), NSLOCTEXT("BlackStatic", "VitalsHeader", "Field State"), 19, FLinearColor(0.96f, 0.96f, 0.96f));
	HealthText = AddText(WidgetTree, VitalsBox, TEXT("HealthText"), FText::GetEmpty(), 14, FLinearColor(0.98f, 0.80f, 0.80f));
	HealthBar = AddBar(WidgetTree, VitalsBox, TEXT("HealthBar"), FLinearColor(0.85f, 0.18f, 0.18f));
	HungerText = AddText(WidgetTree, VitalsBox, TEXT("HungerText"), FText::GetEmpty(), 14, FLinearColor(0.98f, 0.90f, 0.72f));
	HungerBar = AddBar(WidgetTree, VitalsBox, TEXT("HungerBar"), FLinearColor(0.84f, 0.60f, 0.10f));
	StaminaText = AddText(WidgetTree, VitalsBox, TEXT("StaminaText"), FText::GetEmpty(), 14, FLinearColor(0.82f, 0.98f, 0.82f));
	StaminaBar = AddBar(WidgetTree, VitalsBox, TEXT("StaminaBar"), FLinearColor(0.18f, 0.74f, 0.24f));
	StressText = AddText(WidgetTree, VitalsBox, TEXT("StressText"), FText::GetEmpty(), 14, FLinearColor(0.98f, 0.72f, 0.72f));
	StressBar = AddBar(WidgetTree, VitalsBox, TEXT("StressBar"), FLinearColor(0.74f, 0.18f, 0.18f));

	InteractionPromptBorder = MakePanel(WidgetTree, TEXT("InteractionPromptBorder"), FLinearColor(0.04f, 0.04f, 0.04f, 0.75f));
	if (UCanvasPanelSlot* CanvasSlot = RootCanvas->AddChildToCanvas(InteractionPromptBorder))
	{
		CanvasSlot->SetAnchors(FAnchors(0.5f, 1.0f));
		CanvasSlot->SetAlignment(FVector2D(0.5f, 1.0f));
		CanvasSlot->SetPosition(FVector2D(0.0f, -26.0f));
		CanvasSlot->SetSize(FVector2D(520.0f, 62.0f));
	}
	InteractionPromptText = MakeWidget<UTextBlock>(WidgetTree, TEXT("InteractionPromptText"));
	SetTextFontSize(InteractionPromptText, 15);
	InteractionPromptText->SetColorAndOpacity(FSlateColor(FLinearColor(0.95f, 0.95f, 0.95f)));
	InteractionPromptText->SetAutoWrapText(true);
	InteractionPromptText->SetJustification(ETextJustify::Center);
	InteractionPromptBorder->SetContent(InteractionPromptText);

	BackpackPanel = MakePanel(WidgetTree, TEXT("BackpackPanel"), FLinearColor(0.01f, 0.01f, 0.01f, 0.92f));
	if (UCanvasPanelSlot* CanvasSlot = RootCanvas->AddChildToCanvas(BackpackPanel))
	{
		CanvasSlot->SetAnchors(FAnchors(1.0f, 0.5f));
		CanvasSlot->SetAlignment(FVector2D(1.0f, 0.5f));
		CanvasSlot->SetPosition(FVector2D(-22.0f, 0.0f));
		CanvasSlot->SetSize(FVector2D(430.0f, 620.0f));
	}
	BackpackPanel->SetVisibility(ESlateVisibility::Collapsed);

	UVerticalBox* BackpackBox = MakeWidget<UVerticalBox>(WidgetTree, TEXT("BackpackBox"));
	BackpackPanel->SetContent(BackpackBox);
	BackpackImage = MakeWidget<UImage>(WidgetTree, TEXT("BackpackImage"));
	if (UTexture2D* BackpackTexture = LoadObject<UTexture2D>(nullptr, TEXT("/Game/UI/Inventory/Items/Images/IMG_Backpack.IMG_Backpack")))
	{
		BackpackImage->SetBrushFromTexture(BackpackTexture, true);
		if (UVerticalBoxSlot* VerticalSlot = BackpackBox->AddChildToVerticalBox(BackpackImage))
		{
			VerticalSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 10.0f));
			VerticalSlot->SetHorizontalAlignment(HAlign_Left);
		}
	}

	BackpackHeaderText = AddText(WidgetTree, BackpackBox, TEXT("BackpackHeaderText"), NSLOCTEXT("BlackStatic", "BackpackHeader", "Backpack"), 22, FLinearColor(0.96f, 0.96f, 0.96f));
	BackpackSummaryText = AddText(WidgetTree, BackpackBox, TEXT("BackpackSummaryText"), FText::GetEmpty(), 14, FLinearColor(0.83f, 0.89f, 0.94f));
	BackpackHintText = AddText(WidgetTree, BackpackBox, TEXT("BackpackHintText"), FText::GetEmpty(), 13, FLinearColor(0.86f, 0.78f, 0.70f));
	BackpackItemsScrollBox = MakeWidget<UScrollBox>(WidgetTree, TEXT("BackpackItemsScrollBox"));
	if (UVerticalBoxSlot* VerticalSlot = BackpackBox->AddChildToVerticalBox(BackpackItemsScrollBox))
	{
		VerticalSlot->SetPadding(FMargin(0.0f, 10.0f, 0.0f, 0.0f));
	}
}

void UBSPhase0HUDWidget::RefreshHud()
{
	if (UBSSurvivorStateComponent* SurvivorState = ResolveSurvivorState())
	{
		RefreshVitals(SurvivorState->GetVitals());
	}

	if (UBSSettlementSubsystem* SettlementSubsystem = ResolveSettlementSubsystem())
	{
		RefreshSettlement(SettlementSubsystem->GetSettlementState());
	}

	RefreshObjectiveAndOnboarding();
	RefreshInteractionPrompt();
	RefreshInventoryPanel();
}

void UBSPhase0HUDWidget::RefreshVitals(const FBSSurvivorVitals& Vitals)
{
	if (HealthText)
	{
		HealthText->SetText(FText::FromString(FString::Printf(TEXT("Health %d / 100"), FMath::RoundToInt(Vitals.Health))));
	}
	if (HungerText)
	{
		HungerText->SetText(FText::FromString(FString::Printf(TEXT("Hunger %d%%"), FMath::RoundToInt(Vitals.Hunger))));
	}
	if (StaminaText)
	{
		StaminaText->SetText(FText::FromString(FString::Printf(TEXT("Stamina %d%%"), FMath::RoundToInt(Vitals.Stamina))));
	}
	if (StressText)
	{
		const FString PenaltySuffix = Vitals.BountyPenaltySecondsRemaining > 0.0f
			? FString::Printf(TEXT(" | Raider penalty %ds"), FMath::CeilToInt(Vitals.BountyPenaltySecondsRemaining))
			: FString();
		StressText->SetText(FText::FromString(FString::Printf(TEXT("Stress %d%%%s"), FMath::RoundToInt(Vitals.Stress), *PenaltySuffix)));
	}

	if (HealthBar)
	{
		HealthBar->SetPercent(FMath::Clamp(Vitals.Health / 100.0f, 0.0f, 1.0f));
	}
	if (HungerBar)
	{
		HungerBar->SetPercent(FMath::Clamp(Vitals.Hunger / 100.0f, 0.0f, 1.0f));
	}
	if (StaminaBar)
	{
		StaminaBar->SetPercent(FMath::Clamp(Vitals.Stamina / 100.0f, 0.0f, 1.0f));
	}
	if (StressBar)
	{
		StressBar->SetPercent(FMath::Clamp(Vitals.Stress / 100.0f, 0.0f, 1.0f));
	}

	if (StressOverlay)
	{
		const float StressStrength = FMath::Clamp((Vitals.BountyPenaltySecondsRemaining > 0.0f ? 100.0f : Vitals.Stress) / 100.0f, 0.0f, 1.0f);
		StressOverlay->SetBrushColor(FLinearColor(0.20f, 0.01f, 0.01f, 0.08f + (0.24f * StressStrength)));
	}
}

void UBSPhase0HUDWidget::RefreshSettlement(const FBSSettlementState& SettlementState)
{
	if (SettlementSummaryText)
	{
		SettlementSummaryText->SetText(FText::FromString(FString::Printf(
			TEXT("USD %d | Reputation %.0f | Trust %.0f | Unlock %d"),
			SettlementState.USD,
			SettlementState.Reputation,
			SettlementState.VendorTrust,
			SettlementState.UnlockTier)));
	}

	if (SettlementStatusText)
	{
		SettlementStatusText->SetText(FText::FromString(FString::Printf(
			TEXT("Power %d | Water %d | Infrastructure %d | Persistent stash %d items%s"),
			SettlementState.Power,
			SettlementState.Water,
			SettlementState.Infrastructure,
			CountTotalItems(SettlementState.PersistentStash),
			SettlementState.bClassifiedRaider ? TEXT(" | Raider flagged") : TEXT(""))));
	}
}

void UBSPhase0HUDWidget::RefreshObjectiveAndOnboarding()
{
	if (ObjectiveTitleText)
	{
		ObjectiveTitleText->SetText(BuildObjectiveTitle());
	}

	if (ObjectiveStepText)
	{
		ObjectiveStepText->SetText(BuildObjectiveStep());
	}

	if (ObjectiveHintText)
	{
		ObjectiveHintText->SetText(BuildObjectiveHint());
	}
}

void UBSPhase0HUDWidget::RefreshInteractionPrompt()
{
	const FText PromptText = BuildInteractionPromptText();
	if (InteractionPromptText)
	{
		InteractionPromptText->SetText(PromptText);
	}

	if (InteractionPromptBorder)
	{
		InteractionPromptBorder->SetVisibility(PromptText.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	}
}

void UBSPhase0HUDWidget::RefreshInventoryPanel()
{
	if (BackpackPanel)
	{
		BackpackPanel->SetVisibility(bBackpackVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	if (BackpackHintText)
	{
		BackpackHintText->SetText(NSLOCTEXT("BlackStatic", "BackpackHint", "Tab / I closes the backpack. Carried gear is lost on death, stash survives."));
	}

	if (const FString InventorySignature = BuildInventorySignature(); InventorySignature != LastInventorySignature)
	{
		LastInventorySignature = InventorySignature;
		RebuildInventoryList();
	}
}

void UBSPhase0HUDWidget::RebuildInventoryList()
{
	if (!BackpackItemsScrollBox || !BackpackSummaryText)
	{
		return;
	}

	BackpackItemsScrollBox->ClearChildren();

	const UBSInventoryComponent* InventoryComponent = ResolveInventory();
	const UBSSettlementSubsystem* SettlementSubsystem = ResolveSettlementSubsystem();
	const TArray<FBSItemStack> CarriedItems = InventoryComponent ? InventoryComponent->GetItems() : TArray<FBSItemStack>();
	const FBSSettlementState SettlementState = SettlementSubsystem ? SettlementSubsystem->GetSettlementState() : FBSSettlementState();

	BackpackSummaryText->SetText(FText::FromString(FString::Printf(
		TEXT("Carried %d item(s) | Stash %d item(s) | USD %d"),
		CountTotalItems(CarriedItems),
		CountTotalItems(SettlementState.PersistentStash),
		SettlementState.USD)));

	if (CarriedItems.Num() == 0)
	{
		UTextBlock* EmptyText = MakeWidget<UTextBlock>(WidgetTree, TEXT("BackpackEmptyText"));
		SetTextFontSize(EmptyText, 14);
		EmptyText->SetAutoWrapText(true);
		EmptyText->SetColorAndOpacity(FSlateColor(FLinearColor(0.85f, 0.85f, 0.85f)));
		EmptyText->SetText(NSLOCTEXT("BlackStatic", "BackpackEmpty", "Your backpack is empty. Search the zone, recover parts, or pull from stash later."));
		BackpackItemsScrollBox->AddChild(EmptyText);
	}
	else
	{
		for (int32 Index = 0; Index < CarriedItems.Num(); ++Index)
		{
			const FBSItemStack& Stack = CarriedItems[Index];
			const UBSItemDefinition* ItemDefinition = Stack.ItemDefinition.LoadSynchronous();
			const FText ItemName = ItemDefinition && !ItemDefinition->DisplayName.IsEmpty()
				? ItemDefinition->DisplayName
				: FText::FromName(!Stack.ItemId.IsNone() ? Stack.ItemId : FName(TEXT("Unknown")));

			UTextBlock* ItemRow = MakeWidget<UTextBlock>(WidgetTree, *FString::Printf(TEXT("BackpackItem_%d"), Index));
			SetTextFontSize(ItemRow, 14);
			ItemRow->SetAutoWrapText(true);
			ItemRow->SetColorAndOpacity(FSlateColor(FLinearColor(0.95f, 0.95f, 0.95f)));
			ItemRow->SetText(FText::FromString(FString::Printf(
				TEXT("%s x%d%s"),
				*ItemName.ToString(),
				Stack.Quantity,
				(ItemDefinition && ItemDefinition->bQuestCritical) ? TEXT(" | Objective critical") : TEXT(""))));
			BackpackItemsScrollBox->AddChild(ItemRow);
		}
	}

	UTextBlock* Divider = MakeWidget<UTextBlock>(WidgetTree, TEXT("StashDividerText"));
	SetTextFontSize(Divider, 15);
	Divider->SetColorAndOpacity(FSlateColor(FLinearColor(0.78f, 0.86f, 0.96f)));
	Divider->SetText(NSLOCTEXT("BlackStatic", "StashDivider", "Persistent stash summary"));
	BackpackItemsScrollBox->AddChild(Divider);

	if (SettlementState.PersistentStash.Num() == 0)
	{
		UTextBlock* StashEmpty = MakeWidget<UTextBlock>(WidgetTree, TEXT("StashEmptyText"));
		SetTextFontSize(StashEmpty, 13);
		StashEmpty->SetAutoWrapText(true);
		StashEmpty->SetColorAndOpacity(FSlateColor(FLinearColor(0.76f, 0.82f, 0.87f)));
		StashEmpty->SetText(NSLOCTEXT("BlackStatic", "StashEmpty", "No saved stash items yet. Deposit at the settlement box to bank supplies."));
		BackpackItemsScrollBox->AddChild(StashEmpty);
	}
	else
	{
		for (int32 Index = 0; Index < SettlementState.PersistentStash.Num(); ++Index)
		{
			const FBSItemStack& Stack = SettlementState.PersistentStash[Index];
			const UBSItemDefinition* ItemDefinition = Stack.ItemDefinition.LoadSynchronous();
			const FText ItemName = ItemDefinition && !ItemDefinition->DisplayName.IsEmpty()
				? ItemDefinition->DisplayName
				: FText::FromName(!Stack.ItemId.IsNone() ? Stack.ItemId : FName(TEXT("Unknown")));

			UTextBlock* ItemRow = MakeWidget<UTextBlock>(WidgetTree, *FString::Printf(TEXT("StashItem_%d"), Index));
			SetTextFontSize(ItemRow, 13);
			ItemRow->SetAutoWrapText(true);
			ItemRow->SetColorAndOpacity(FSlateColor(FLinearColor(0.76f, 0.82f, 0.87f)));
			ItemRow->SetText(FText::FromString(FString::Printf(TEXT("%s x%d"), *ItemName.ToString(), Stack.Quantity)));
			BackpackItemsScrollBox->AddChild(ItemRow);
		}
	}
}

FText UBSPhase0HUDWidget::BuildObjectiveTitle() const
{
	const UBSTaskComponent* TaskComponent = ResolveTaskComponent();
	if (!TaskComponent || !TaskComponent->GetActiveTask())
	{
		return NSLOCTEXT("BlackStatic", "HudOnboardingTitle", "Settlement Onboarding");
	}

	const UBSTaskDefinition* TaskDefinition = TaskComponent->GetActiveTask();
	return TaskDefinition->DisplayName.IsEmpty() ? FText::FromName(TaskDefinition->ResolveTaskId()) : TaskDefinition->DisplayName;
}

FText UBSPhase0HUDWidget::BuildObjectiveStep() const
{
	const UBSTaskComponent* TaskComponent = ResolveTaskComponent();
	const UBSInventoryComponent* InventoryComponent = ResolveInventory();
	if (!TaskComponent || !TaskComponent->GetActiveTask())
	{
		return NSLOCTEXT("BlackStatic", "HudOnboardingStep", "Accept Water & Power Restoration at the settlement task board, then move out to recover the missing parts.");
	}

	switch (TaskComponent->GetObjectiveState())
	{
	case EBSObjectiveState::InProgress:
		if (InventoryComponent && InventoryComponent->HasRequiredItems(TaskComponent->GetActiveTask()->RequiredItems))
		{
			return NSLOCTEXT("BlackStatic", "HudStepReadyForInstall", "You have the battery and filter. Reach the maintenance site and install them.");
		}
		return NSLOCTEXT("BlackStatic", "HudStepRecoverParts", "Search the zone for a battery and filter while keeping noise under control.");

	case EBSObjectiveState::SiteCompleted:
	case EBSObjectiveState::ExtractReady:
		return NSLOCTEXT("BlackStatic", "HudStepExtract", "Utilities are restored. Break contact and return to the extraction point inside the settlement.");

	case EBSObjectiveState::Completed:
		return NSLOCTEXT("BlackStatic", "HudStepComplete", "Run completed. Deposit useful items, bank progress, and accept another task.");

	case EBSObjectiveState::Failed:
		return NSLOCTEXT("BlackStatic", "HudStepFailed", "The survivor was lost. Your carried gear is gone, but the stash and settlement progress remain.");

	default:
		return NSLOCTEXT("BlackStatic", "HudStepFallback", "Regroup in the settlement and prepare for the next run.");
	}
}

FText UBSPhase0HUDWidget::BuildObjectiveHint() const
{
	const UBSSurvivorStateComponent* SurvivorState = ResolveSurvivorState();
	const UBSTaskComponent* TaskComponent = ResolveTaskComponent();
	if (SurvivorState && SurvivorState->IsUnderBountyPenalty())
	{
		return NSLOCTEXT("BlackStatic", "HudHintPenalty", "Player-kill penalties max stress, blur vision, and slash movement. Stay disciplined.");
	}

	if (SurvivorState && SurvivorState->GetVitals().Stress >= 70.0f)
	{
		return NSLOCTEXT("BlackStatic", "HudHintStress", "Stress is high. Slow down, crouch, and create distance before taking another fight.");
	}

	if (!TaskComponent || !TaskComponent->GetActiveTask())
	{
		return NSLOCTEXT("BlackStatic", "HudHintOnboarding", "Use E on the task board to start the loop. The stash is persistent; the backpack is not.");
	}

	if (TaskComponent->IsReadyForExtraction())
	{
		return NSLOCTEXT("BlackStatic", "HudHintExtraction", "Extraction is the handoff point where the settlement gets paid and progression advances.");
	}

	return NSLOCTEXT("BlackStatic", "HudHintNoise", "Sprinting, fighting, and object interactions all create noise. Treat silence like ammunition.");
}

FText UBSPhase0HUDWidget::BuildInteractionPromptText() const
{
	const ABSPhase0Character* Character = ResolveCharacter();
	if (!Character)
	{
		return FText::GetEmpty();
	}

	if (bBackpackVisible)
	{
		return NSLOCTEXT("BlackStatic", "HudPromptCloseBackpack", "[Tab / I] Close backpack and return to the world.");
	}

	if (const FText CurrentPrompt = Character->GetCurrentInteractionPrompt(); !CurrentPrompt.IsEmpty())
	{
		return FText::Format(NSLOCTEXT("BlackStatic", "HudPromptAction", "[E] {0}"), CurrentPrompt);
	}

	const UBSTaskComponent* TaskComponent = ResolveTaskComponent();
	if (!TaskComponent || !TaskComponent->GetActiveTask())
	{
		return NSLOCTEXT("BlackStatic", "HudPromptGuideTaskBoard", "Find the task board in the settlement and press E to accept your first field run.");
	}

	return NSLOCTEXT("BlackStatic", "HudPromptGuideBackpack", "[Tab / I] Open backpack to review carried items and stash progress.");
}

FString UBSPhase0HUDWidget::BuildInventorySignature() const
{
	FString Signature;

	if (const UBSInventoryComponent* InventoryComponent = ResolveInventory())
	{
		for (const FBSItemStack& Stack : InventoryComponent->GetItems())
		{
			Signature += FString::Printf(TEXT("C:%s:%d|"), *Stack.ItemId.ToString(), Stack.Quantity);
		}
	}

	if (const UBSSettlementSubsystem* SettlementSubsystem = ResolveSettlementSubsystem())
	{
		const FBSSettlementState& SettlementState = SettlementSubsystem->GetSettlementState();
		Signature += FString::Printf(TEXT("USD:%d|REP:%d|"), SettlementState.USD, FMath::RoundToInt(SettlementState.Reputation));
		for (const FBSItemStack& Stack : SettlementState.PersistentStash)
		{
			Signature += FString::Printf(TEXT("S:%s:%d|"), *Stack.ItemId.ToString(), Stack.Quantity);
		}
	}

	return Signature;
}

ABSPhase0Character* UBSPhase0HUDWidget::ResolveCharacter() const
{
	return GetOwningPlayerPawn<ABSPhase0Character>();
}

UBSInventoryComponent* UBSPhase0HUDWidget::ResolveInventory() const
{
	if (ABSPhase0Character* Character = ResolveCharacter())
	{
		return Character->GetInventoryComponent();
	}

	return nullptr;
}

UBSTaskComponent* UBSPhase0HUDWidget::ResolveTaskComponent() const
{
	if (ABSPhase0Character* Character = ResolveCharacter())
	{
		return Character->GetTaskComponent();
	}

	return nullptr;
}

UBSSurvivorStateComponent* UBSPhase0HUDWidget::ResolveSurvivorState() const
{
	if (ABSPhase0Character* Character = ResolveCharacter())
	{
		return Character->GetSurvivorStateComponent();
	}

	return nullptr;
}

UBSSettlementSubsystem* UBSPhase0HUDWidget::ResolveSettlementSubsystem() const
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		return GameInstance->GetSubsystem<UBSSettlementSubsystem>();
	}

	return nullptr;
}
