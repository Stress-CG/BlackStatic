#include "Phase0/Components/BSNoiseListenerComponent.h"

#include "Phase0/Systems/BSNoiseSubsystem.h"

UBSNoiseListenerComponent::UBSNoiseListenerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UBSNoiseListenerComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetWorld())
	{
		CachedNoiseSubsystem = GetWorld()->GetSubsystem<UBSNoiseSubsystem>();
		if (CachedNoiseSubsystem.IsValid())
		{
			NoiseDelegateHandle = CachedNoiseSubsystem->OnNoiseEvent().AddUObject(this, &UBSNoiseListenerComponent::HandleNoiseEvent);
		}
	}
}

void UBSNoiseListenerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (CachedNoiseSubsystem.IsValid())
	{
		CachedNoiseSubsystem->OnNoiseEvent().Remove(NoiseDelegateHandle);
	}

	Super::EndPlay(EndPlayReason);
}

bool UBSNoiseListenerComponent::GetMostRelevantNoise(FBSNoiseEvent& OutNoiseEvent) const
{
	PruneExpiredEvents();
	if (HeardNoiseEvents.Num() == 0)
	{
		return false;
	}

	const FBSNoiseEvent* BestEvent = nullptr;
	for (const FBSNoiseEvent& Event : HeardNoiseEvents)
	{
		if (!BestEvent || Event.Loudness > BestEvent->Loudness || Event.GameTimeSeconds > BestEvent->GameTimeSeconds)
		{
			BestEvent = &Event;
		}
	}

	if (!BestEvent)
	{
		return false;
	}

	OutNoiseEvent = *BestEvent;
	return true;
}

const TArray<FBSNoiseEvent>& UBSNoiseListenerComponent::GetHeardNoiseEvents() const
{
	PruneExpiredEvents();
	return HeardNoiseEvents;
}

void UBSNoiseListenerComponent::HandleNoiseEvent(const FBSNoiseEvent& NoiseEvent)
{
	if (!GetOwner())
	{
		return;
	}

	if (bIgnoreFriendlyNoise && NoiseEvent.InstigatorFaction == ListeningFaction)
	{
		return;
	}

	if (NoiseEvent.Loudness < MinimumLoudness)
	{
		return;
	}

	const float DistanceToNoise = FVector::Distance(GetOwner()->GetActorLocation(), NoiseEvent.Location);
	if (DistanceToNoise > (NoiseEvent.RadiusUnits * HearingRadiusMultiplier))
	{
		return;
	}

	HeardNoiseEvents.Add(NoiseEvent);
	OnNoiseHeard.Broadcast(NoiseEvent);
	PruneExpiredEvents();
}

void UBSNoiseListenerComponent::PruneExpiredEvents() const
{
	if (!GetWorld())
	{
		return;
	}

	const float Cutoff = GetWorld()->GetTimeSeconds() - MemorySeconds;
	HeardNoiseEvents.RemoveAll([Cutoff](const FBSNoiseEvent& Event)
	{
		return Event.GameTimeSeconds < Cutoff;
	});
}
