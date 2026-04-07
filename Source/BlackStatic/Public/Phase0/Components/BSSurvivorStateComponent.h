#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Phase0/BlackStaticPhase0Types.h"
#include "BSSurvivorStateComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBSSurvivorVitalsChangedSignature, const FBSSurvivorVitals&, Vitals);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FBSSurvivorDeathSignature);

UCLASS(ClassGroup = (BlackStatic), BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class BLACKSTATIC_API UBSSurvivorStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBSSurvivorStateComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(BlueprintAssignable, Category = "Vitals")
	FBSSurvivorVitalsChangedSignature OnVitalsChanged;

	UPROPERTY(BlueprintAssignable, Category = "Vitals")
	FBSSurvivorDeathSignature OnDeath;

	UFUNCTION(BlueprintCallable, Category = "Vitals")
	float ApplyWorldDamage(float DamageAmount);

	UFUNCTION(BlueprintCallable, Category = "Vitals")
	void ApplyStress(float StressAmount);

	UFUNCTION(BlueprintCallable, Category = "Vitals")
	bool ConsumeStamina(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Vitals")
	void RestoreForFreshSurvivor();

	UFUNCTION(BlueprintCallable, Category = "Vitals")
	void SetSprinting(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Vitals")
	void SetHunted(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Vitals")
	void ApplyPlayerKillPenalty();

	UFUNCTION(BlueprintPure, Category = "Vitals")
	const FBSSurvivorVitals& GetVitals() const;

	UFUNCTION(BlueprintPure, Category = "Vitals")
	float GetMovementSpeedMultiplier() const;

	UFUNCTION(BlueprintPure, Category = "Vitals")
	float GetVisionDisruptionStrength() const;

	UFUNCTION(BlueprintPure, Category = "Vitals")
	bool IsAlive() const;

	UFUNCTION(BlueprintPure, Category = "Vitals")
	bool IsUnderBountyPenalty() const;

private:
	void BroadcastVitalsChanged();
	void HandleDeath();

	UPROPERTY(EditAnywhere, Category = "Vitals")
	float MaxHealth = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Vitals")
	float MaxStamina = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Vitals")
	float MaxStress = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Vitals")
	float HungerGainPerSecond = 0.35f;

	UPROPERTY(EditAnywhere, Category = "Vitals")
	float HungerDamageThreshold = 85.0f;

	UPROPERTY(EditAnywhere, Category = "Vitals")
	float HungerDamagePerSecond = 2.0f;

	UPROPERTY(EditAnywhere, Category = "Vitals")
	float StressRecoveryPerSecond = 5.0f;

	UPROPERTY(EditAnywhere, Category = "Vitals")
	float HuntedStressPerSecond = 8.0f;

	UPROPERTY(EditAnywhere, Category = "Vitals")
	float SprintStaminaDrainPerSecond = 16.0f;

	UPROPERTY(EditAnywhere, Category = "Vitals")
	float StaminaRecoveryPerSecond = 12.0f;

	UPROPERTY(EditAnywhere, Category = "Vitals")
	float PlayerKillPenaltySeconds = 60.0f;

	UPROPERTY(VisibleAnywhere, Category = "Vitals")
	FBSSurvivorVitals CurrentVitals;

	bool bIsSprinting = false;
	bool bIsHunted = false;
};
