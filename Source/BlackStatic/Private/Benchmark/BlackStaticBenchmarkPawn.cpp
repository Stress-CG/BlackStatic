#include "Benchmark/BlackStaticBenchmarkPawn.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

ABSBenchmarkPawn::ABSBenchmarkPawn()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(SceneRoot);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh.Succeeded())
	{
		MeshComponent->SetStaticMesh(SphereMesh.Object);
		MeshComponent->SetRelativeScale3D(FVector(0.25f));
	}
}

void ABSBenchmarkPawn::ResetRoute(const FVector& StartLocation, const TArray<FVector>& InWaypoints, float InSpeed, float InAcceptanceRadius)
{
	SetActorLocation(StartLocation);
	Waypoints = InWaypoints;
	ExecutedPath.Reset();
	ExecutedPath.Add(StartLocation);
	CurrentWaypointIndex = 0;
	MoveSpeedUnitsPerSecond = InSpeed;
	AcceptanceRadius = InAcceptanceRadius;
}

bool ABSBenchmarkPawn::AdvanceAlongRoute(float DeltaSeconds, float& OutDistanceAdvanced)
{
	OutDistanceAdvanced = 0.0f;
	if (!HasRoute() || HasFinishedRoute())
	{
		return false;
	}

	FVector CurrentLocation = GetActorLocation();
	FVector TargetLocation = Waypoints[CurrentWaypointIndex];
	const FVector Delta = TargetLocation - CurrentLocation;
	const float DistanceToTarget = Delta.Size();

	if (DistanceToTarget <= AcceptanceRadius)
	{
		SetActorLocation(TargetLocation);
		CurrentLocation = TargetLocation;
		ExecutedPath.Add(CurrentLocation);
		++CurrentWaypointIndex;

		if (HasFinishedRoute())
		{
			return true;
		}

		TargetLocation = Waypoints[CurrentWaypointIndex];
	}

	const FVector MoveDirection = (TargetLocation - CurrentLocation).GetSafeNormal();
	const float StepDistance = FMath::Min((TargetLocation - CurrentLocation).Size(), MoveSpeedUnitsPerSecond * DeltaSeconds);
	const FVector NewLocation = CurrentLocation + (MoveDirection * StepDistance);
	SetActorLocation(NewLocation);
	ExecutedPath.Add(NewLocation);
	OutDistanceAdvanced = StepDistance;

	if ((TargetLocation - NewLocation).Size() <= AcceptanceRadius)
	{
		SetActorLocation(TargetLocation);
		ExecutedPath.Add(TargetLocation);
		++CurrentWaypointIndex;
	}

	return HasFinishedRoute();
}

bool ABSBenchmarkPawn::HasFinishedRoute() const
{
	return Waypoints.Num() == 0 || CurrentWaypointIndex >= Waypoints.Num();
}

bool ABSBenchmarkPawn::HasRoute() const
{
	return Waypoints.Num() > 0;
}

float ABSBenchmarkPawn::GetDistanceToGoal() const
{
	if (!HasRoute() || HasFinishedRoute())
	{
		return 0.0f;
	}

	return FVector::Distance(GetActorLocation(), Waypoints.Last());
}

const TArray<FVector>& ABSBenchmarkPawn::GetExecutedPath() const
{
	return ExecutedPath;
}

