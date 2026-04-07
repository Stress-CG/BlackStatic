#include "Phase0/Framework/BSPhase0PlayerController.h"

#include "Blueprint/UserWidget.h"
#include "Phase0/UI/BSPhase0HUDWidget.h"

ABSPhase0PlayerController::ABSPhase0PlayerController()
{
	HUDWidgetClass = UBSPhase0HUDWidget::StaticClass();
}

void ABSPhase0PlayerController::BeginPlay()
{
	Super::BeginPlay();

	EnsureHUDWidget();
	HandleControlledPawnChanged();
}

void ABSPhase0PlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	HandleControlledPawnChanged();
}

void ABSPhase0PlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();
	HandleControlledPawnChanged();
}

void ABSPhase0PlayerController::ToggleBackpack()
{
	SetBackpackVisible(!bBackpackVisible);
}

void ABSPhase0PlayerController::SetBackpackVisible(const bool bVisible)
{
	bBackpackVisible = bVisible;

	EnsureHUDWidget();
	if (HUDWidget)
	{
		HUDWidget->SetBackpackVisible(bBackpackVisible);
	}

	if (!IsLocalController())
	{
		return;
	}

	if (bBackpackVisible)
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		if (HUDWidget)
		{
			if (TSharedPtr<SWidget> FocusWidget = HUDWidget->GetBackpackFocusWidget(); FocusWidget.IsValid())
			{
				InputMode.SetWidgetToFocus(FocusWidget);
			}
		}

		SetInputMode(InputMode);
		bShowMouseCursor = true;
		SetIgnoreLookInput(true);
		SetIgnoreMoveInput(true);
	}
	else
	{
		SetInputMode(FInputModeGameOnly());
		bShowMouseCursor = false;
		SetIgnoreLookInput(false);
		SetIgnoreMoveInput(false);
	}
}

bool ABSPhase0PlayerController::IsBackpackVisible() const
{
	return bBackpackVisible;
}

void ABSPhase0PlayerController::ShowPhase0Message(const FText& Message, const FLinearColor& Color, const float DurationSeconds)
{
	EnsureHUDWidget();
	if (HUDWidget)
	{
		HUDWidget->PushNotification(Message, Color, DurationSeconds);
	}
}

void ABSPhase0PlayerController::EnsureHUDWidget()
{
	if (!IsLocalController() || HUDWidget || !HUDWidgetClass)
	{
		return;
	}

	HUDWidget = CreateWidget<UBSPhase0HUDWidget>(this, HUDWidgetClass);
	if (HUDWidget)
	{
		HUDWidget->AddToViewport(10);
		HUDWidget->SetBackpackVisible(bBackpackVisible);
	}
}

void ABSPhase0PlayerController::HandleControlledPawnChanged()
{
	EnsureHUDWidget();
	SetBackpackVisible(false);

	if (HUDWidget)
	{
		HUDWidget->RefreshNow();
	}

	ShowPhase0Message(
		NSLOCTEXT("BlackStatic", "Phase0OnboardingSpawn", "Use E to interact, press Tab to open your backpack, and start at the task board."),
		FLinearColor(0.72f, 0.90f, 1.0f),
		6.0f);
}
