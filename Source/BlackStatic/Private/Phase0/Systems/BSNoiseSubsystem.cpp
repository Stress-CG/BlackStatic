#include "Phase0/Systems/BSNoiseSubsystem.h"

void UBSNoiseSubsystem::EmitNoise(FBSNoiseEvent NoiseEvent)
{
	PruneExpiredEvents();
	NoiseEvent.GameTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	RecentNoiseEvents.Add(NoiseEvent);
	NoiseEventDelegate.Broadcast(NoiseEvent);
}

const TArray<FBSNoiseEvent>& UBSNoiseSubsystem::GetRecentNoiseEvents() const
{
	return RecentNoiseEvents;
}

FBSNoiseEventNativeSignature& UBSNoiseSubsystem::OnNoiseEvent()
{
	return NoiseEventDelegate;
}

void UBSNoiseSubsystem::PruneExpiredEvents()
{
	if (!GetWorld())
	{
		return;
	}

	const float CutoffTime = GetWorld()->GetTimeSeconds() - NoiseMemorySeconds;
	RecentNoiseEvents.RemoveAll([CutoffTime](const FBSNoiseEvent& Event)
	{
		return Event.GameTimeSeconds < CutoffTime;
	});
}
