#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Phase0/BlackStaticPhase0Types.h"
#include "BSNoiseListenerComponent.generated.h"

class UBSNoiseSubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBSNoiseHeardSignature, const FBSNoiseEvent&, NoiseEvent);

UCLASS(ClassGroup = (BlackStatic), BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class BLACKSTATIC_API UBSNoiseListenerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBSNoiseListenerComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(BlueprintAssignable, Category = "Noise")
	FBSNoiseHeardSignature OnNoiseHeard;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
	EBSFaction ListeningFaction = EBSFaction::Neutral;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
	bool bIgnoreFriendlyNoise = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
	float HearingRadiusMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
	float MinimumLoudness = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
	float MemorySeconds = 6.0f;

	UFUNCTION(BlueprintPure, Category = "Noise")
	bool GetMostRelevantNoise(FBSNoiseEvent& OutNoiseEvent) const;

	UFUNCTION(BlueprintPure, Category = "Noise")
	const TArray<FBSNoiseEvent>& GetHeardNoiseEvents() const;

private:
	void HandleNoiseEvent(const FBSNoiseEvent& NoiseEvent);
	void PruneExpiredEvents() const;

	UPROPERTY()
	mutable TArray<FBSNoiseEvent> HeardNoiseEvents;

	TWeakObjectPtr<UBSNoiseSubsystem> CachedNoiseSubsystem;
	FDelegateHandle NoiseDelegateHandle;
};
