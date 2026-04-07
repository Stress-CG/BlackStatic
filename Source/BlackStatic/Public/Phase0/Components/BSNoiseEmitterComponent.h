#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Phase0/BlackStaticPhase0Types.h"
#include "BSNoiseEmitterComponent.generated.h"

UCLASS(ClassGroup = (BlackStatic), BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class BLACKSTATIC_API UBSNoiseEmitterComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBSNoiseEmitterComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
	EBSFaction EmittedFaction = EBSFaction::Neutral;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
	float BaseMovementNoiseRadius = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
	float BaseCombatNoiseRadius = 2600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
	float BaseToolNoiseRadius = 1400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
	float BaseDoorNoiseRadius = 900.0f;

	UFUNCTION(BlueprintCallable, Category = "Noise")
	void EmitNoise(EBSNoiseType NoiseType, float Loudness, FName NoiseTag = NAME_None, FVector OverrideLocation = FVector::ZeroVector);

	UFUNCTION(BlueprintCallable, Category = "Noise")
	void EmitMovementNoise(float Speed, float MaxWalkSpeed, bool bIsCrouched, bool bIsSprinting);

private:
	float GetBaseRadiusForNoiseType(EBSNoiseType NoiseType) const;
};
