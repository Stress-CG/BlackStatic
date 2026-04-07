#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BSInfectedAIController.generated.h"

class ABSInfectedCharacter;
class ABSPhase0Character;

UENUM()
enum class EBSInfectedAIState : uint8
{
	Roaming,
	Investigating,
	Chasing
};

UCLASS()
class BLACKSTATIC_API ABSInfectedAIController : public AAIController
{
	GENERATED_BODY()

public:
	ABSInfectedAIController();

	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaSeconds) override;

private:
	void UpdateBehavior();
	void UpdateRoam();
	void SetCurrentTarget(ABSPhase0Character* NewTarget);
	ABSPhase0Character* FindVisibleTarget() const;

	UPROPERTY()
	TObjectPtr<ABSInfectedCharacter> ControlledInfected;

	UPROPERTY()
	TObjectPtr<ABSPhase0Character> CurrentTarget;

	UPROPERTY()
	TObjectPtr<ABSPhase0Character> PreviouslyHuntedTarget;

	EBSInfectedAIState CurrentState = EBSInfectedAIState::Roaming;
	FVector SpawnOrigin = FVector::ZeroVector;
	float DecisionAccumulator = 0.0f;
	FVector CurrentRoamDestination = FVector::ZeroVector;
};
