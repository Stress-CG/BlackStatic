#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BSPhase0PlayerController.generated.h"

class UBSPhase0HUDWidget;

UCLASS()
class BLACKSTATIC_API ABSPhase0PlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ABSPhase0PlayerController();

	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnRep_Pawn() override;

	UFUNCTION(BlueprintCallable, Category = "Phase0|HUD")
	void ToggleBackpack();

	UFUNCTION(BlueprintCallable, Category = "Phase0|HUD")
	void SetBackpackVisible(bool bVisible);

	UFUNCTION(BlueprintPure, Category = "Phase0|HUD")
	bool IsBackpackVisible() const;

	void ShowPhase0Message(const FText& Message, const FLinearColor& Color = FLinearColor::White, float DurationSeconds = 3.0f);

private:
	void EnsureHUDWidget();
	void HandleControlledPawnChanged();

	UPROPERTY(EditDefaultsOnly, Category = "Phase0|HUD")
	TSubclassOf<UBSPhase0HUDWidget> HUDWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UBSPhase0HUDWidget> HUDWidget;

	bool bBackpackVisible = false;
};
