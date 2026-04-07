#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Phase0/BlackStaticPhase0Types.h"
#include "Templates/SharedPointer.h"
#include "BSPhase0HUDWidget.generated.h"

class UBorder;
class UCanvasPanel;
class UImage;
class UProgressBar;
class UScrollBox;
class UTextBlock;
class UUserWidget;
class UVerticalBox;
class SWidget;

UCLASS()
class BLACKSTATIC_API UBSPhase0HUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	void SetBackpackVisible(bool bVisible);
	bool IsBackpackVisible() const;
	TSharedPtr<SWidget> GetBackpackFocusWidget() const;

	void PushNotification(const FText& Message, const FLinearColor& Color, float DurationSeconds);
	void RefreshNow();

private:
	void BuildHud();
	void RefreshHud();
	void RefreshVitals(const FBSSurvivorVitals& Vitals);
	void RefreshSettlement(const FBSSettlementState& SettlementState);
	void RefreshObjectiveAndOnboarding();
	void RefreshInteractionPrompt();
	void RefreshInventoryPanel();
	void RebuildInventoryList();

	FText BuildObjectiveTitle() const;
	FText BuildObjectiveStep() const;
	FText BuildObjectiveHint() const;
	FText BuildInteractionPromptText() const;
	FString BuildInventorySignature() const;

	class ABSPhase0Character* ResolveCharacter() const;
	class UBSInventoryComponent* ResolveInventory() const;
	class UBSTaskComponent* ResolveTaskComponent() const;
	class UBSSurvivorStateComponent* ResolveSurvivorState() const;
	class UBSSettlementSubsystem* ResolveSettlementSubsystem() const;

	UPROPERTY(Transient)
	TObjectPtr<UCanvasPanel> RootCanvas;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> SettlementPanel;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> SettlementSummaryText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> SettlementStatusText;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> ObjectivePanel;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ObjectiveTitleText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ObjectiveStepText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ObjectiveHintText;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> InteractionPromptBorder;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> InteractionPromptText;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> NotificationBorder;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> NotificationText;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> StressOverlay;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> VitalsPanel;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> HealthText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> HungerText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> StaminaText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> StressText;

	UPROPERTY(Transient)
	TObjectPtr<UProgressBar> HealthBar;

	UPROPERTY(Transient)
	TObjectPtr<UProgressBar> HungerBar;

	UPROPERTY(Transient)
	TObjectPtr<UProgressBar> StaminaBar;

	UPROPERTY(Transient)
	TObjectPtr<UProgressBar> StressBar;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> BackpackPanel;

	UPROPERTY(Transient)
	TObjectPtr<UCanvasPanel> BackpackCanvas;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> BackpackDetailsPanel;

	UPROPERTY(Transient)
	TObjectPtr<UImage> BackpackImage;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> BackpackHeaderText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> BackpackSummaryText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> BackpackHintText;

	UPROPERTY(Transient)
	TObjectPtr<UScrollBox> BackpackItemsScrollBox;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> LegacyBackpackShellWidget;

	UPROPERTY(Transient)
	FText ActiveNotification;

	UPROPERTY(Transient)
	FLinearColor ActiveNotificationColor = FLinearColor::White;

	UPROPERTY(Transient)
	float NotificationSecondsRemaining = 0.0f;

	UPROPERTY(Transient)
	float HudRefreshAccumulator = 0.0f;

	UPROPERTY(Transient)
	FString LastInventorySignature;

	bool bBackpackVisible = false;
};
