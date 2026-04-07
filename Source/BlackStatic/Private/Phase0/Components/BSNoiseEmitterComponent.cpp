#include "Phase0/Components/BSNoiseEmitterComponent.h"

#include "Phase0/BlackStaticPhase0Statics.h"
#include "Phase0/Systems/BSNoiseSubsystem.h"

UBSNoiseEmitterComponent::UBSNoiseEmitterComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UBSNoiseEmitterComponent::EmitNoise(const EBSNoiseType NoiseType, const float Loudness, const FName NoiseTag, const FVector OverrideLocation)
{
	if (!GetWorld() || !GetOwner())
	{
		return;
	}

	UBSNoiseSubsystem* NoiseSubsystem = GetWorld()->GetSubsystem<UBSNoiseSubsystem>();
	if (!NoiseSubsystem)
	{
		return;
	}

	FBSNoiseEvent Event;
	Event.Location = OverrideLocation.IsNearlyZero() ? GetOwner()->GetActorLocation() : OverrideLocation;
	Event.Loudness = FMath::Clamp(Loudness, 0.0f, 2.0f);
	Event.RadiusUnits = GetBaseRadiusForNoiseType(NoiseType) * Event.Loudness;
	Event.NoiseType = NoiseType;
	Event.InstigatorFaction = EmittedFaction;
	Event.NoiseTag = NoiseTag;
	Event.InstigatorActor = GetOwner();
	NoiseSubsystem->EmitNoise(Event);
}

void UBSNoiseEmitterComponent::EmitMovementNoise(const float Speed, const float MaxWalkSpeed, const bool bIsCrouched, const bool bIsSprinting)
{
	const float Loudness = UBSPhase0Statics::ComputeMovementNoiseLoudness(Speed, MaxWalkSpeed, bIsCrouched, bIsSprinting);
	if (Loudness > 0.05f)
	{
		EmitNoise(EBSNoiseType::Movement, Loudness, TEXT("Movement"));
	}
}

float UBSNoiseEmitterComponent::GetBaseRadiusForNoiseType(const EBSNoiseType NoiseType) const
{
	switch (NoiseType)
	{
	case EBSNoiseType::Combat:
		return BaseCombatNoiseRadius;
	case EBSNoiseType::Tool:
	case EBSNoiseType::Objective:
		return BaseToolNoiseRadius;
	case EBSNoiseType::Door:
		return BaseDoorNoiseRadius;
	case EBSNoiseType::Movement:
	default:
		return BaseMovementNoiseRadius;
	}
}
