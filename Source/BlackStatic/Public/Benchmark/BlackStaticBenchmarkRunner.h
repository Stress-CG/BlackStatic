#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Benchmark/BSTypes.h"
#include "BlackStaticBenchmarkRunner.generated.h"

class ABSBenchmarkController;
class ABSBenchmarkPawn;
class SWidget;
class STextBlock;
class UBlackStaticBenchmarkScenarioData;

USTRUCT()
struct FBSQueuedVisualRun
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UBlackStaticBenchmarkScenarioData> Scenario;

	UPROPERTY()
	EBSBenchmarkMode Mode = EBSBenchmarkMode::Baseline;
};

UCLASS()
class BLACKSTATIC_API ABSBenchmarkRunner : public AActor
{
	GENERATED_BODY()

public:
	ABSBenchmarkRunner();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Benchmark")
	TObjectPtr<UBlackStaticBenchmarkScenarioData> Scenario;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Benchmark")
	TArray<TObjectPtr<UBlackStaticBenchmarkScenarioData>> ScenarioSequence;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Benchmark")
	bool bAutoRunOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Benchmark")
	bool bCycleScenarioSequence = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Benchmark")
	bool bDiscoverScenariosWhenSequenceEmpty = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Benchmark")
	bool bRunBaselineAndImprovedBackToBack = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Benchmark")
	float PauseBetweenQueuedRunsSeconds = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Benchmark")
	bool bWriteArtifactsAfterVisualRuns = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Benchmark")
	FString VisualRunOutputDirectory = TEXT("Saved/DemoRuns");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Benchmark")
	EBSBenchmarkMode AutoRunMode = EBSBenchmarkMode::Baseline;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Benchmark")
	bool bDrawDebug = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Benchmark")
	float DebugDrawDuration = 20.0f;

	UFUNCTION(BlueprintCallable, Category = "Benchmark")
	void StartScenario(UBlackStaticBenchmarkScenarioData* InScenario, EBSBenchmarkMode Mode);

	UFUNCTION(BlueprintCallable, Category = "Benchmark")
	FBSBenchmarkMetrics GetLastMetrics() const;

	static FBSBenchmarkMetrics RunSynchronousBenchmark(const UBlackStaticBenchmarkScenarioData* Scenario, EBSBenchmarkMode Mode, const FString& OutputDirectory, bool bWriteArtifacts = true, bool bLogSummary = true);
	static bool WriteAggregateSummary(const TArray<FBSBenchmarkMetrics>& Runs, const FString& OutputDirectory, FString* OutSummaryPath = nullptr);
	static bool WriteMetricsBundle(const FBSBenchmarkMetrics& Metrics, const FString& OutputDirectory, FString* OutMetricsPath = nullptr);

private:
	static FBSBenchmarkMetrics ExecuteBenchmark(const UBlackStaticBenchmarkScenarioData* Scenario, const FBSPlanRequest& PlanRequest, bool bAllowRecoveryReplan);

	TArray<TObjectPtr<UBlackStaticBenchmarkScenarioData>> GetResolvedScenarioSequence() const;
	void QueueScenarioRuns(const TArray<TObjectPtr<UBlackStaticBenchmarkScenarioData>>& InScenarios);
	void TryStartNextQueuedRun();
	void FlushVisualRunArtifacts();
	FBSPlanRequest BuildVisualPlanRequest(EBSBenchmarkMode Mode) const;
	void EnsureVisualActors();
	void BeginVisualRun(UBlackStaticBenchmarkScenarioData* InScenario, EBSBenchmarkMode Mode, bool bQueueFollowUpMode);
	void AdvanceVisualRun(float DeltaSeconds);
	void TriggerRecoveryReplan();
	void FinishVisualRun(const FString& FailureReason = FString());
	void UpdateOverlayText();
	void DrawScenarioDebug() const;

	UPROPERTY(Transient)
	TObjectPtr<ABSBenchmarkPawn> BenchmarkPawn;

	UPROPERTY(Transient)
	TObjectPtr<ABSBenchmarkController> BenchmarkController;

	bool bIsRunning = false;
	bool bHasRecoveryReplanned = false;
	float PendingQueuedRunDelaySeconds = 0.0f;
	TOptional<EBSBenchmarkMode> QueuedMode;
	FBSPlanRequest ActivePlanRequest;
	FBSPlanResult ActivePlanResult;
	FBSBenchmarkMetrics LastMetrics;
	TArray<FBSBenchmarkMetrics> CompletedVisualRuns;
	TArray<FBSQueuedVisualRun> PendingVisualRuns;
	float CurrentElapsedSeconds = 0.0f;
	float TimeWithoutProgress = 0.0f;
	float LastDistanceToGoal = TNumericLimits<float>::Max();
	TArray<FVector> StallMarkers;
	TSharedPtr<STextBlock> SummaryTextBlock;
	TSharedPtr<SWidget> SummaryOverlay;
};
