#include "Phase0/Components/BSSurvivorStateComponent.h"

UBSSurvivorStateComponent::UBSSurvivorStateComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UBSSurvivorStateComponent::BeginPlay()
{
	Super::BeginPlay();
	RestoreForFreshSurvivor();
}

void UBSSurvivorStateComponent::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!CurrentVitals.bAlive)
	{
		return;
	}

	CurrentVitals.Hunger = FMath::Clamp(CurrentVitals.Hunger + (HungerGainPerSecond * DeltaTime), 0.0f, 100.0f);
	if (CurrentVitals.Hunger >= HungerDamageThreshold)
	{
		CurrentVitals.Health = FMath::Clamp(CurrentVitals.Health - (HungerDamagePerSecond * DeltaTime), 0.0f, MaxHealth);
	}

	if (bIsSprinting)
	{
		CurrentVitals.Stamina = FMath::Clamp(CurrentVitals.Stamina - (SprintStaminaDrainPerSecond * DeltaTime), 0.0f, MaxStamina);
		if (CurrentVitals.Stamina <= KINDA_SMALL_NUMBER)
		{
			bIsSprinting = false;
		}
	}
	else
	{
		CurrentVitals.Stamina = FMath::Clamp(CurrentVitals.Stamina + (StaminaRecoveryPerSecond * DeltaTime), 0.0f, MaxStamina);
	}

	if (CurrentVitals.BountyPenaltySecondsRemaining > 0.0f)
	{
		CurrentVitals.BountyPenaltySecondsRemaining = FMath::Max(0.0f, CurrentVitals.BountyPenaltySecondsRemaining - DeltaTime);
		CurrentVitals.Stress = MaxStress;
	}
	else if (bIsHunted)
	{
		CurrentVitals.Stress = FMath::Clamp(CurrentVitals.Stress + (HuntedStressPerSecond * DeltaTime), 0.0f, MaxStress);
	}
	else
	{
		CurrentVitals.Stress = FMath::Clamp(CurrentVitals.Stress - (StressRecoveryPerSecond * DeltaTime), 0.0f, MaxStress);
	}

	if (CurrentVitals.Health <= KINDA_SMALL_NUMBER)
	{
		HandleDeath();
	}

	BroadcastVitalsChanged();
}

float UBSSurvivorStateComponent::ApplyWorldDamage(const float DamageAmount)
{
	if (!CurrentVitals.bAlive || DamageAmount <= 0.0f)
	{
		return 0.0f;
	}

	CurrentVitals.Health = FMath::Clamp(CurrentVitals.Health - DamageAmount, 0.0f, MaxHealth);
	ApplyStress(DamageAmount * 0.6f);
	if (CurrentVitals.Health <= KINDA_SMALL_NUMBER)
	{
		HandleDeath();
	}

	BroadcastVitalsChanged();
	return DamageAmount;
}

void UBSSurvivorStateComponent::ApplyStress(const float StressAmount)
{
	if (!CurrentVitals.bAlive || StressAmount <= 0.0f)
	{
		return;
	}

	CurrentVitals.Stress = FMath::Clamp(CurrentVitals.Stress + StressAmount, 0.0f, MaxStress);
	BroadcastVitalsChanged();
}

bool UBSSurvivorStateComponent::ConsumeStamina(const float Amount)
{
	if (CurrentVitals.Stamina < Amount)
	{
		return false;
	}

	CurrentVitals.Stamina = FMath::Clamp(CurrentVitals.Stamina - Amount, 0.0f, MaxStamina);
	BroadcastVitalsChanged();
	return true;
}

void UBSSurvivorStateComponent::RestoreForFreshSurvivor()
{
	CurrentVitals.Health = MaxHealth;
	CurrentVitals.Hunger = 0.0f;
	CurrentVitals.Stamina = MaxStamina;
	CurrentVitals.Stress = 0.0f;
	CurrentVitals.bAlive = true;
	CurrentVitals.BountyPenaltySecondsRemaining = 0.0f;
	bIsSprinting = false;
	bIsHunted = false;
	BroadcastVitalsChanged();
}

void UBSSurvivorStateComponent::SetSprinting(const bool bEnabled)
{
	bIsSprinting = bEnabled && CurrentVitals.bAlive;
}

void UBSSurvivorStateComponent::SetHunted(const bool bEnabled)
{
	bIsHunted = bEnabled && CurrentVitals.bAlive;
}

void UBSSurvivorStateComponent::ApplyPlayerKillPenalty()
{
	CurrentVitals.Stress = MaxStress;
	CurrentVitals.BountyPenaltySecondsRemaining = PlayerKillPenaltySeconds;
	BroadcastVitalsChanged();
}

const FBSSurvivorVitals& UBSSurvivorStateComponent::GetVitals() const
{
	return CurrentVitals;
}

float UBSSurvivorStateComponent::GetMovementSpeedMultiplier() const
{
	float Multiplier = CurrentVitals.BountyPenaltySecondsRemaining > 0.0f ? 0.5f : 1.0f;
	if (CurrentVitals.Stress >= 80.0f)
	{
		Multiplier *= 0.8f;
	}

	return Multiplier;
}

float UBSSurvivorStateComponent::GetVisionDisruptionStrength() const
{
	if (CurrentVitals.BountyPenaltySecondsRemaining > 0.0f)
	{
		return 1.0f;
	}

	return FMath::Clamp(CurrentVitals.Stress / MaxStress, 0.0f, 1.0f);
}

bool UBSSurvivorStateComponent::IsAlive() const
{
	return CurrentVitals.bAlive;
}

bool UBSSurvivorStateComponent::IsUnderBountyPenalty() const
{
	return CurrentVitals.BountyPenaltySecondsRemaining > 0.0f;
}

void UBSSurvivorStateComponent::BroadcastVitalsChanged()
{
	OnVitalsChanged.Broadcast(CurrentVitals);
}

void UBSSurvivorStateComponent::HandleDeath()
{
	if (!CurrentVitals.bAlive)
	{
		return;
	}

	CurrentVitals.bAlive = false;
	CurrentVitals.Health = 0.0f;
	OnDeath.Broadcast();
}
