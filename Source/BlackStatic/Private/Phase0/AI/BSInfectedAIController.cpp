#include "Phase0/AI/BSInfectedAIController.h"

#include "EngineUtils.h"
#include "NavigationSystem.h"
#include "Phase0/Actors/BSInfectedCharacter.h"
#include "Phase0/Actors/BSPhase0Character.h"
#include "Phase0/Components/BSNoiseListenerComponent.h"
#include "Phase0/Components/BSSurvivorStateComponent.h"

ABSInfectedAIController::ABSInfectedAIController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ABSInfectedAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	ControlledInfected = Cast<ABSInfectedCharacter>(InPawn);
	SpawnOrigin = InPawn ? InPawn->GetActorLocation() : FVector::ZeroVector;
}

void ABSInfectedAIController::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	DecisionAccumulator += DeltaSeconds;
	if (DecisionAccumulator >= 0.2f)
	{
		DecisionAccumulator = 0.0f;
		UpdateBehavior();
	}

	if (ControlledInfected && CurrentTarget)
	{
		if (FVector::Distance(ControlledInfected->GetActorLocation(), CurrentTarget->GetActorLocation()) <= ControlledInfected->AttackRange)
		{
			StopMovement();
			ControlledInfected->TryAttackTarget(CurrentTarget);
		}
	}
}

void ABSInfectedAIController::UpdateBehavior()
{
	if (!ControlledInfected)
	{
		return;
	}

	if (ABSPhase0Character* VisibleTarget = FindVisibleTarget())
	{
		SetCurrentTarget(VisibleTarget);
		CurrentState = EBSInfectedAIState::Chasing;
		MoveToActor(VisibleTarget, ControlledInfected->AttackRange * 0.75f, true);
		return;
	}

	SetCurrentTarget(nullptr);

	FBSNoiseEvent HeardNoise;
	if (ControlledInfected->NoiseListenerComponent && ControlledInfected->NoiseListenerComponent->GetMostRelevantNoise(HeardNoise))
	{
		CurrentState = EBSInfectedAIState::Investigating;
		MoveToLocation(HeardNoise.Location, 60.0f, true);
		return;
	}

	CurrentState = EBSInfectedAIState::Roaming;
	UpdateRoam();
}

void ABSInfectedAIController::UpdateRoam()
{
	if (!ControlledInfected)
	{
		return;
	}

	const bool bNeedsNewDestination = CurrentRoamDestination.IsNearlyZero() || FVector::Distance(ControlledInfected->GetActorLocation(), CurrentRoamDestination) <= 125.0f;
	if (!bNeedsNewDestination)
	{
		return;
	}

	if (UNavigationSystemV1* NavigationSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()))
	{
		FNavLocation NavLocation;
		if (NavigationSystem->GetRandomReachablePointInRadius(SpawnOrigin, ControlledInfected->RoamRadius, NavLocation))
		{
			CurrentRoamDestination = NavLocation.Location;
			MoveToLocation(CurrentRoamDestination, 75.0f, true);
		}
	}
}

void ABSInfectedAIController::SetCurrentTarget(ABSPhase0Character* NewTarget)
{
	if (PreviouslyHuntedTarget && PreviouslyHuntedTarget != NewTarget && PreviouslyHuntedTarget->GetSurvivorStateComponent())
	{
		PreviouslyHuntedTarget->GetSurvivorStateComponent()->SetHunted(false);
	}

	CurrentTarget = NewTarget;
	PreviouslyHuntedTarget = NewTarget;
	if (CurrentTarget && CurrentTarget->GetSurvivorStateComponent())
	{
		CurrentTarget->GetSurvivorStateComponent()->SetHunted(true);
	}
}

ABSPhase0Character* ABSInfectedAIController::FindVisibleTarget() const
{
	if (!ControlledInfected || !GetWorld())
	{
		return nullptr;
	}

	ABSPhase0Character* BestTarget = nullptr;
	float BestDistance = TNumericLimits<float>::Max();

	for (TActorIterator<ABSPhase0Character> It(GetWorld()); It; ++It)
	{
		ABSPhase0Character* Candidate = *It;
		if (!Candidate || !Candidate->GetSurvivorStateComponent() || !Candidate->GetSurvivorStateComponent()->IsAlive())
		{
			continue;
		}

		float SightRadius = ControlledInfected->SightRadius;
		if (Candidate->GetSurvivorStateComponent()->IsUnderBountyPenalty())
		{
			SightRadius *= 1.35f;
		}

		const float Distance = FVector::Distance(Candidate->GetActorLocation(), ControlledInfected->GetActorLocation());
		if (Distance > SightRadius || !LineOfSightTo(Candidate))
		{
			continue;
		}

		if (Distance < BestDistance)
		{
			BestDistance = Distance;
			BestTarget = Candidate;
		}
	}

	return BestTarget;
}
