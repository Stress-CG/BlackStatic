#pragma once

#include "CoreMinimal.h"
#include "BSTypes.generated.h"

UENUM(BlueprintType)
enum class EBSBenchmarkMode : uint8
{
	Baseline UMETA(DisplayName = "Baseline"),
	Improved UMETA(DisplayName = "Improved")
};

USTRUCT(BlueprintType)
struct FBSGridRect
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Benchmark")
	FIntPoint Min = FIntPoint::ZeroValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Benchmark")
	FIntPoint Max = FIntPoint::ZeroValue;

	void Normalize()
	{
		if (Min.X > Max.X)
		{
			Swap(Min.X, Max.X);
		}

		if (Min.Y > Max.Y)
		{
			Swap(Min.Y, Max.Y);
		}
	}
};

USTRUCT(BlueprintType)
struct FBSPlanRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Benchmark")
	FString ScenarioId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Benchmark")
	FString PlannerLabel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Benchmark")
	EBSBenchmarkMode Mode = EBSBenchmarkMode::Baseline;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Benchmark")
	int32 GridWidth = 32;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Benchmark")
	int32 GridHeight = 32;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Benchmark")
	float CellSize = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Benchmark")
	FVector WorldOrigin = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Benchmark")
	FIntPoint StartCell = FIntPoint(1, 1);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Benchmark")
	FIntPoint GoalCell = FIntPoint(10, 10);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Benchmark")
	TArray<FBSGridRect> BlockedAreas;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Benchmark")
	int32 Seed = 1337;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Benchmark")
	bool bAllowDiagonal = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Benchmark")
	float ClearanceWeight = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Benchmark")
	int32 ClearanceInflation = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Benchmark")
	float AgentSpeedUnitsPerSecond = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Benchmark")
	float TurnPenaltySeconds = 0.08f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Benchmark")
	float WaypointAcceptanceRadius = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Benchmark")
	float StallDistanceEpsilon = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Benchmark")
	float StallTimeoutSeconds = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Benchmark")
	int32 MaxReplans = 0;
};

USTRUCT(BlueprintType)
struct FBSPlanResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Benchmark")
	bool bSuccess = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Benchmark")
	FString FailureReason;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Benchmark")
	FString PlannerLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Benchmark")
	EBSBenchmarkMode Mode = EBSBenchmarkMode::Baseline;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Benchmark")
	int32 ExpandedNodes = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Benchmark")
	double PlanCost = 0.0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Benchmark")
	double PlannerRuntimeMs = 0.0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Benchmark")
	float PathLengthUnits = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Benchmark")
	float MinimumClearanceUnits = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Benchmark")
	FString PathHash;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Benchmark")
	TArray<FIntPoint> Cells;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Benchmark")
	TArray<FVector> WorldPoints;
};

USTRUCT(BlueprintType)
struct FBSBenchmarkMetrics
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Benchmark")
	FString ScenarioId;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Benchmark")
	FString PlannerLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Benchmark")
	EBSBenchmarkMode Mode = EBSBenchmarkMode::Baseline;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Benchmark")
	int32 Seed = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Benchmark")
	bool bSuccess = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Benchmark")
	FString FailureReason;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Benchmark")
	double PlannerRuntimeMs = 0.0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Benchmark")
	float PlannedPathLength = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Benchmark")
	float ExecutedLength = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Benchmark")
	float TimeToGoalSeconds = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Benchmark")
	float MinimumClearanceUnits = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Benchmark")
	int32 TurnCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Benchmark")
	int32 ReplanCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Benchmark")
	int32 StallEventsCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Benchmark")
	int32 CollisionCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Benchmark")
	int32 IllegalTraversalCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Benchmark")
	FString PathHash;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Benchmark")
	TArray<FIntPoint> Cells;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Benchmark")
	TArray<FVector> WorldPoints;
};

BLACKSTATIC_API FString LexToString(EBSBenchmarkMode Mode);

