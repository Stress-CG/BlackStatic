#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Benchmark/BSTypes.h"
#include "BlackStaticBenchmarkScenarioData.generated.h"

class UWorld;

UCLASS(BlueprintType)
class BLACKSTATIC_API UBlackStaticBenchmarkScenarioData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Benchmark")
	FName ScenarioId = TEXT("Scenario");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Benchmark")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Benchmark")
	bool bIncludeInCommandletSuite = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Benchmark")
	int32 GridWidth = 32;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Benchmark")
	int32 GridHeight = 18;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Benchmark")
	float CellSize = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Benchmark")
	FVector LocalWorldOrigin = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Benchmark")
	FIntPoint StartCell = FIntPoint(1, 1);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Benchmark")
	FIntPoint GoalCell = FIntPoint(10, 10);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Benchmark")
	TArray<FBSGridRect> BlockedAreas;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Benchmark")
	int32 Seed = 1337;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Benchmark")
	float BaselineAgentSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Benchmark")
	float ImprovedAgentSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Benchmark")
	float TurnPenaltySeconds = 0.18f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Benchmark")
	float WaypointAcceptanceRadius = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Benchmark")
	float StallDistanceEpsilon = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Benchmark")
	float StallTimeoutSeconds = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Benchmark")
	float ImprovedClearanceWeight = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Benchmark")
	float RecoveryClearanceWeight = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Benchmark")
	int32 RecoveryClearanceInflation = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Benchmark")
	TSoftObjectPtr<UWorld> BenchmarkMap;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Benchmark")
	TSoftObjectPtr<UWorld> ShowcaseMap;

	FBSPlanRequest MakePlanRequest(EBSBenchmarkMode Mode, const FVector& AdditionalWorldOffset = FVector::ZeroVector) const;
};
