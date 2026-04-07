#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Phase0/BlackStaticPhase0Types.h"
#include "BSNoiseSubsystem.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FBSNoiseEventNativeSignature, const FBSNoiseEvent&);

UCLASS()
class BLACKSTATIC_API UBSNoiseSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	void EmitNoise(FBSNoiseEvent NoiseEvent);
	const TArray<FBSNoiseEvent>& GetRecentNoiseEvents() const;
	FBSNoiseEventNativeSignature& OnNoiseEvent();

private:
	void PruneExpiredEvents();

	UPROPERTY()
	TArray<FBSNoiseEvent> RecentNoiseEvents;

	FBSNoiseEventNativeSignature NoiseEventDelegate;

	UPROPERTY(EditDefaultsOnly, Category = "Noise")
	float NoiseMemorySeconds = 6.0f;
};
