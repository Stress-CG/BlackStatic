#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Phase0/BlackStaticPhase0Types.h"
#include "BSPhase0Character.generated.h"

class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class UBSInventoryComponent;
class UBSNoiseEmitterComponent;
class UBSSurvivorStateComponent;
class UBSTaskComponent;
class AActor;
struct FInputActionValue;

UCLASS(Blueprintable)
class BLACKSTATIC_API ABSPhase0Character : public ACharacter
{
	GENERATED_BODY()

public:
	ABSPhase0Character();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCameraComponent> FirstPersonCameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBSInventoryComponent> InventoryComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBSSurvivorStateComponent> SurvivorStateComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBSNoiseEmitterComponent> NoiseEmitterComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBSTaskComponent> TaskComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase0")
	EBSFaction Faction = EBSFaction::Settlements;

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void TryInteract();

	UFUNCTION(BlueprintCallable, Category = "Phase0")
	void SetSprintEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Phase0")
	void ReportCombatNoise(float Loudness = 1.0f);

	UFUNCTION(BlueprintPure, Category = "Phase0")
	UBSInventoryComponent* GetInventoryComponent() const;

	UFUNCTION(BlueprintPure, Category = "Phase0")
	UBSSurvivorStateComponent* GetSurvivorStateComponent() const;

	UFUNCTION(BlueprintPure, Category = "Phase0")
	UBSTaskComponent* GetTaskComponent() const;

	UFUNCTION(BlueprintPure, Category = "Interaction")
	FText GetCurrentInteractionPrompt() const;

	UFUNCTION(BlueprintPure, Category = "Interaction")
	AActor* GetFocusedInteractableActor() const;

protected:
	UFUNCTION()
	void HandleSurvivorDeath();

private:
	void ApplyInputMappings();
	void UpdateMovementSpeedFromVitals();
	void UpdateFocusedInteractable();
	void UpdateStressCameraEffects(float DeltaSeconds);
	void InputMove(const FInputActionValue& Value);
	void InputLook(const FInputActionValue& Value);
	void InputJumpStarted();
	void InputJumpStopped();
	void InputToggleCrouch();
	void InputSprintStarted();
	void InputSprintStopped();
	void InputToggleInventory();

	UPROPERTY(EditAnywhere, Category = "Phase0")
	float IntentionalWalkSpeed = 260.0f;

	UPROPERTY(EditAnywhere, Category = "Phase0")
	float SprintSpeed = 430.0f;

	UPROPERTY(EditAnywhere, Category = "Phase0")
	float InteractionDistance = 250.0f;

	UPROPERTY(EditAnywhere, Category = "Phase0")
	float MovementNoiseIntervalSeconds = 0.2f;

	UPROPERTY()
	TObjectPtr<UInputMappingContext> DefaultInputMapping;

	UPROPERTY()
	TObjectPtr<UInputMappingContext> MouseLookMapping;

	UPROPERTY()
	TObjectPtr<UInputAction> MoveInputAction;

	UPROPERTY()
	TObjectPtr<UInputAction> LookInputAction;

	UPROPERTY()
	TObjectPtr<UInputAction> MouseLookInputAction;

	UPROPERTY()
	TObjectPtr<UInputAction> JumpInputAction;

	UPROPERTY()
	TObjectPtr<UInputAction> CrouchInputAction;

	UPROPERTY()
	TObjectPtr<UInputAction> RunInputAction;

	float MovementNoiseAccumulator = 0.0f;
	float StressVisualTimeSeconds = 0.0f;
	bool bSprintRequested = false;

	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> FocusedInteractableActor;

	UPROPERTY(Transient)
	FText FocusedInteractionPrompt;

	FVector BaseCameraRelativeLocation = FVector::ZeroVector;
	float BaseCameraFOV = 90.0f;
};
