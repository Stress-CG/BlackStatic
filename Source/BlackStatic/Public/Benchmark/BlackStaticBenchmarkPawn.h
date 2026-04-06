#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "BlackStaticBenchmarkPawn.generated.h"

class UStaticMeshComponent;
class USceneComponent;

UCLASS()
class BLACKSTATIC_API ABSBenchmarkPawn : public APawn
{
	GENERATED_BODY()

public:
	ABSBenchmarkPawn();

	void ResetRoute(const FVector& StartLocation, const TArray<FVector>& InWaypoints, float InSpeed, float InAcceptanceRadius);
	bool AdvanceAlongRoute(float DeltaSeconds, float& OutDistanceAdvanced);
	bool HasFinishedRoute() const;
	bool HasRoute() const;
	float GetDistanceToGoal() const;
	const TArray<FVector>& GetExecutedPath() const;

private:
	UPROPERTY(VisibleAnywhere, Category = "Benchmark")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, Category = "Benchmark")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	TArray<FVector> Waypoints;
	TArray<FVector> ExecutedPath;
	int32 CurrentWaypointIndex = 0;
	float MoveSpeedUnitsPerSecond = 600.0f;
	float AcceptanceRadius = 15.0f;
};

