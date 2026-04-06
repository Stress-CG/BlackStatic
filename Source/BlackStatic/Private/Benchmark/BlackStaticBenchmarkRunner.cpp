#include "Benchmark/BlackStaticBenchmarkRunner.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Benchmark/BSAStarPlanner.h"
#include "Benchmark/BSGridUtils.h"
#include "Benchmark/BSThetaStarPlanner.h"
#include "Benchmark/BlackStaticBenchmarkController.h"
#include "Benchmark/BlackStaticBenchmarkPawn.h"
#include "Benchmark/BlackStaticBenchmarkScenarioData.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "HAL/FileManager.h"
#include "JsonObjectConverter.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SWeakWidget.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	IBSPathPlanner& GetPlanner(EBSBenchmarkMode Mode)
	{
		static FBSAStarPlanner BaselinePlanner;
		static FBSThetaStarPlanner ImprovedPlanner;
		return Mode == EBSBenchmarkMode::Baseline ? static_cast<IBSPathPlanner&>(BaselinePlanner) : static_cast<IBSPathPlanner&>(ImprovedPlanner);
	}

	FColor GetModeColor(EBSBenchmarkMode Mode)
	{
		return Mode == EBSBenchmarkMode::Baseline ? FColor::Red : FColor::Green;
	}

	FString MakeMetricsFilename(const FBSBenchmarkMetrics& Metrics)
	{
		return FString::Printf(TEXT("%s_%s.json"), *Metrics.ScenarioId, *LexToString(Metrics.Mode));
	}

	float CalculateWorldPathLength(const TArray<FVector>& Points)
	{
		if (Points.Num() < 2)
		{
			return 0.0f;
		}

		float Total = 0.0f;
		for (int32 Index = 1; Index < Points.Num(); ++Index)
		{
			Total += FVector::Distance(Points[Index - 1], Points[Index]);
		}

		return Total;
	}
}

ABSBenchmarkRunner::ABSBenchmarkRunner()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void ABSBenchmarkRunner::BeginPlay()
{
	Super::BeginPlay();
	EnsureVisualActors();
	UpdateOverlayText();

	if (!bAutoRunOnBeginPlay)
	{
		return;
	}

	if (bCycleScenarioSequence)
	{
		QueueScenarioRuns(GetResolvedScenarioSequence());
		TryStartNextQueuedRun();
	}
	else if (Scenario)
	{
		BeginVisualRun(Scenario, AutoRunMode, bRunBaselineAndImprovedBackToBack && AutoRunMode == EBSBenchmarkMode::Baseline);
	}
}

void ABSBenchmarkRunner::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bIsRunning)
	{
		AdvanceVisualRun(DeltaSeconds);
	}
	else if (PendingVisualRuns.Num() > 0)
	{
		PendingQueuedRunDelaySeconds = FMath::Max(0.0f, PendingQueuedRunDelaySeconds - DeltaSeconds);
		if (PendingQueuedRunDelaySeconds <= 0.0f)
		{
			TryStartNextQueuedRun();
		}
	}
}

void ABSBenchmarkRunner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (SummaryOverlay.IsValid() && GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(SummaryOverlay.ToSharedRef());
	}

	SummaryOverlay.Reset();
	SummaryTextBlock.Reset();

	Super::EndPlay(EndPlayReason);
}

void ABSBenchmarkRunner::StartScenario(UBlackStaticBenchmarkScenarioData* InScenario, EBSBenchmarkMode Mode)
{
	PendingVisualRuns.Reset();
	CompletedVisualRuns.Reset();
	PendingQueuedRunDelaySeconds = 0.0f;
	BeginVisualRun(InScenario, Mode, false);
}

FBSBenchmarkMetrics ABSBenchmarkRunner::GetLastMetrics() const
{
	return LastMetrics;
}

FBSBenchmarkMetrics ABSBenchmarkRunner::RunSynchronousBenchmark(const UBlackStaticBenchmarkScenarioData* ScenarioAsset, EBSBenchmarkMode Mode, const FString& OutputDirectory, bool bWriteArtifacts, bool bLogSummary)
{
	check(ScenarioAsset);
	FBSBenchmarkMetrics Metrics = ExecuteBenchmark(ScenarioAsset, ScenarioAsset->MakePlanRequest(Mode), true);

	if (bWriteArtifacts)
	{
		WriteMetricsBundle(Metrics, OutputDirectory);
		WriteAggregateSummary({ Metrics }, OutputDirectory, nullptr);
	}

	if (bLogSummary)
	{
		UE_LOG(LogTemp, Display, TEXT("[%s][%s] success=%s time=%.3fs length=%.2f planner=%.2fms"),
			*Metrics.ScenarioId,
			*LexToString(Metrics.Mode),
			Metrics.bSuccess ? TEXT("true") : TEXT("false"),
			Metrics.TimeToGoalSeconds,
			Metrics.ExecutedLength,
			Metrics.PlannerRuntimeMs);
	}

	return Metrics;
}

bool ABSBenchmarkRunner::WriteAggregateSummary(const TArray<FBSBenchmarkMetrics>& Runs, const FString& OutputDirectory, FString* OutSummaryPath)
{
	IFileManager::Get().MakeDirectory(*OutputDirectory, true);

	TSharedPtr<FJsonObject> RootObject = MakeShared<FJsonObject>();
	RootObject->SetStringField(TEXT("generated_at"), FDateTime::UtcNow().ToIso8601());

	TArray<TSharedPtr<FJsonValue>> JsonRuns;
	for (const FBSBenchmarkMetrics& Run : Runs)
	{
		TSharedPtr<FJsonObject> RunObject = MakeShared<FJsonObject>();
		if (!FJsonObjectConverter::UStructToJsonObject(FBSBenchmarkMetrics::StaticStruct(), &Run, RunObject.ToSharedRef(), 0, 0))
		{
			return false;
		}

		RunObject->SetStringField(TEXT("mode"), LexToString(Run.Mode));
		RunObject->SetBoolField(TEXT("success"), Run.bSuccess);
		RunObject->SetNumberField(TEXT("time_to_goal_s"), Run.TimeToGoalSeconds);
		RunObject->SetNumberField(TEXT("executed_length"), Run.ExecutedLength);
		RunObject->SetNumberField(TEXT("planner_runtime_ms"), Run.PlannerRuntimeMs);
		RunObject->SetNumberField(TEXT("illegal_traversal_count"), Run.IllegalTraversalCount);
		RunObject->SetNumberField(TEXT("collision_count"), Run.CollisionCount);
		RunObject->SetStringField(TEXT("scenario_id"), Run.ScenarioId);
		JsonRuns.Add(MakeShared<FJsonValueObject>(RunObject));
	}

	RootObject->SetArrayField(TEXT("runs"), JsonRuns);

	FString OutputJson;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputJson);
	if (!FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer))
	{
		return false;
	}

	const FString SummaryPath = FPaths::Combine(OutputDirectory, TEXT("summary.json"));
	if (!FFileHelper::SaveStringToFile(OutputJson, *SummaryPath))
	{
		return false;
	}

	FString CsvContent = TEXT("scenario_id,mode,success,time_to_goal_s,executed_length,planner_runtime_ms,illegal_traversal_count,collision_count,path_hash\n");
	for (const FBSBenchmarkMetrics& Run : Runs)
	{
		CsvContent += FString::Printf(TEXT("%s,%s,%s,%.6f,%.6f,%.6f,%d,%d,%s\n"),
			*Run.ScenarioId,
			*LexToString(Run.Mode),
			Run.bSuccess ? TEXT("true") : TEXT("false"),
			Run.TimeToGoalSeconds,
			Run.ExecutedLength,
			Run.PlannerRuntimeMs,
			Run.IllegalTraversalCount,
			Run.CollisionCount,
			*Run.PathHash);
	}

	if (!FFileHelper::SaveStringToFile(CsvContent, *FPaths::Combine(OutputDirectory, TEXT("summary.csv"))))
	{
		return false;
	}

	if (OutSummaryPath)
	{
		*OutSummaryPath = SummaryPath;
	}

	return true;
}

bool ABSBenchmarkRunner::WriteMetricsBundle(const FBSBenchmarkMetrics& Metrics, const FString& OutputDirectory, FString* OutMetricsPath)
{
	IFileManager::Get().MakeDirectory(*OutputDirectory, true);

	FString OutputJson;
	if (!FJsonObjectConverter::UStructToJsonObjectString(Metrics, OutputJson))
	{
		return false;
	}

	const FString MetricsPath = FPaths::Combine(OutputDirectory, MakeMetricsFilename(Metrics));
	if (!FFileHelper::SaveStringToFile(OutputJson, *MetricsPath))
	{
		return false;
	}

	if (OutMetricsPath)
	{
		*OutMetricsPath = MetricsPath;
	}

	return true;
}

FBSBenchmarkMetrics ABSBenchmarkRunner::ExecuteBenchmark(const UBlackStaticBenchmarkScenarioData* ScenarioAsset, const FBSPlanRequest& PlanRequest, bool bAllowRecoveryReplan)
{
	FBSBenchmarkMetrics Metrics;
	Metrics.ScenarioId = PlanRequest.ScenarioId;
	Metrics.Mode = PlanRequest.Mode;
	Metrics.PlannerLabel = PlanRequest.PlannerLabel;
	Metrics.Seed = PlanRequest.Seed;

	FBSPlanRequest WorkingRequest = PlanRequest;
	FBSPlanResult PlanResult = GetPlanner(WorkingRequest.Mode).Plan(WorkingRequest);
	FBSRuntimeGrid Grid = FBSGridUtils::BuildRuntimeGrid(WorkingRequest);

	if (bAllowRecoveryReplan && WorkingRequest.Mode == EBSBenchmarkMode::Improved)
	{
		const int32 IllegalTraversals = FBSGridUtils::CountIllegalTraversals(Grid, PlanResult.Cells);
		if ((!PlanResult.bSuccess || IllegalTraversals > 0) && ScenarioAsset)
		{
			WorkingRequest.ClearanceInflation += ScenarioAsset->RecoveryClearanceInflation;
			WorkingRequest.ClearanceWeight = ScenarioAsset->RecoveryClearanceWeight;
			PlanResult = GetPlanner(WorkingRequest.Mode).Plan(WorkingRequest);
			Grid = FBSGridUtils::BuildRuntimeGrid(WorkingRequest);
			Metrics.ReplanCount = 1;
		}
	}

	Metrics.bSuccess = PlanResult.bSuccess;
	Metrics.FailureReason = PlanResult.FailureReason;
	Metrics.PlannerRuntimeMs = PlanResult.PlannerRuntimeMs;
	Metrics.PlannedPathLength = PlanResult.PathLengthUnits;
	Metrics.ExecutedLength = PlanResult.PathLengthUnits;
	Metrics.MinimumClearanceUnits = PlanResult.MinimumClearanceUnits;
	Metrics.PathHash = PlanResult.PathHash;
	Metrics.Cells = PlanResult.Cells;
	Metrics.WorldPoints = PlanResult.WorldPoints;
	Metrics.TurnCount = FBSGridUtils::CountDirectionChanges(PlanResult.Cells);
	Metrics.IllegalTraversalCount = FBSGridUtils::CountIllegalTraversals(Grid, PlanResult.Cells);
	Metrics.CollisionCount = 0;
	Metrics.StallEventsCount = 0;

	if (PlanResult.bSuccess)
	{
		const float TravelSeconds = PlanRequest.AgentSpeedUnitsPerSecond > 0.0f ? (Metrics.ExecutedLength / PlanRequest.AgentSpeedUnitsPerSecond) : 0.0f;
		Metrics.TimeToGoalSeconds = TravelSeconds + (static_cast<float>(Metrics.TurnCount) * PlanRequest.TurnPenaltySeconds);
	}

	return Metrics;
}

TArray<TObjectPtr<UBlackStaticBenchmarkScenarioData>> ABSBenchmarkRunner::GetResolvedScenarioSequence() const
{
	TArray<TObjectPtr<UBlackStaticBenchmarkScenarioData>> Result;
	for (UBlackStaticBenchmarkScenarioData* ScenarioAsset : ScenarioSequence)
	{
		if (ScenarioAsset)
		{
			Result.Add(ScenarioAsset);
		}
	}

	if (Result.Num() > 0 || !bDiscoverScenariosWhenSequenceEmpty)
	{
		if (Result.Num() == 0 && Scenario)
		{
			Result.Add(Scenario);
		}
		return Result;
	}

	FAssetRegistryModule& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	FARFilter Filter;
	Filter.PackagePaths.Add(TEXT("/Game/Benchmarks/Scenarios"));
	Filter.ClassPaths.Add(UBlackStaticBenchmarkScenarioData::StaticClass()->GetClassPathName());
	Filter.bRecursivePaths = true;

	TArray<FAssetData> ScenarioAssets;
	AssetRegistry.Get().GetAssets(Filter, ScenarioAssets);
	ScenarioAssets.Sort([](const FAssetData& Left, const FAssetData& Right)
	{
		return Left.AssetName.LexicalLess(Right.AssetName);
	});

	for (const FAssetData& AssetData : ScenarioAssets)
	{
		if (UBlackStaticBenchmarkScenarioData* ScenarioAsset = Cast<UBlackStaticBenchmarkScenarioData>(AssetData.GetAsset()))
		{
			if (ScenarioAsset->bIncludeInCommandletSuite)
			{
				Result.Add(ScenarioAsset);
			}
		}
	}

	if (Result.Num() == 0 && Scenario)
	{
		Result.Add(Scenario);
	}

	return Result;
}

void ABSBenchmarkRunner::QueueScenarioRuns(const TArray<TObjectPtr<UBlackStaticBenchmarkScenarioData>>& InScenarios)
{
	PendingVisualRuns.Reset();
	CompletedVisualRuns.Reset();
	PendingQueuedRunDelaySeconds = 0.0f;

	for (UBlackStaticBenchmarkScenarioData* ScenarioAsset : InScenarios)
	{
		if (!ScenarioAsset)
		{
			continue;
		}

		PendingVisualRuns.Add({ ScenarioAsset, AutoRunMode });
		if (bRunBaselineAndImprovedBackToBack && AutoRunMode == EBSBenchmarkMode::Baseline)
		{
			PendingVisualRuns.Add({ ScenarioAsset, EBSBenchmarkMode::Improved });
		}
	}
}

void ABSBenchmarkRunner::TryStartNextQueuedRun()
{
	if (bIsRunning || PendingVisualRuns.Num() == 0)
	{
		if (!bIsRunning && PendingVisualRuns.Num() == 0)
		{
			FlushVisualRunArtifacts();
			UpdateOverlayText();
		}
		return;
	}

	const FBSQueuedVisualRun NextRun = PendingVisualRuns[0];
	PendingVisualRuns.RemoveAt(0);
	BeginVisualRun(NextRun.Scenario, NextRun.Mode, false);
}

void ABSBenchmarkRunner::FlushVisualRunArtifacts()
{
	if (!bWriteArtifactsAfterVisualRuns || CompletedVisualRuns.Num() == 0)
	{
		return;
	}

	const FString BaseDir = FPaths::IsRelative(VisualRunOutputDirectory)
		? FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() / VisualRunOutputDirectory)
		: VisualRunOutputDirectory;
	const FString SessionDir = FPaths::Combine(BaseDir, FDateTime::Now().ToString(TEXT("%Y%m%d-%H%M%S")));

	for (const FBSBenchmarkMetrics& Metrics : CompletedVisualRuns)
	{
		WriteMetricsBundle(Metrics, SessionDir);
	}

	WriteAggregateSummary(CompletedVisualRuns, SessionDir, nullptr);
}

FBSPlanRequest ABSBenchmarkRunner::BuildVisualPlanRequest(EBSBenchmarkMode Mode) const
{
	check(Scenario);
	return Scenario->MakePlanRequest(Mode, GetActorLocation());
}

void ABSBenchmarkRunner::EnsureVisualActors()
{
	if (!GetWorld())
	{
		return;
	}

	if (!BenchmarkPawn)
	{
		BenchmarkPawn = GetWorld()->SpawnActor<ABSBenchmarkPawn>(ABSBenchmarkPawn::StaticClass(), GetActorLocation(), FRotator::ZeroRotator);
	}

	if (!BenchmarkController)
	{
		BenchmarkController = GetWorld()->SpawnActor<ABSBenchmarkController>(ABSBenchmarkController::StaticClass(), GetActorLocation(), FRotator::ZeroRotator);
	}

	if (BenchmarkPawn && BenchmarkController && BenchmarkController->GetPawn() != BenchmarkPawn)
	{
		BenchmarkController->Possess(BenchmarkPawn);
	}

	if (!SummaryOverlay.IsValid() && GEngine && GEngine->GameViewport)
	{
		SAssignNew(SummaryTextBlock, STextBlock)
			.Text(FText::FromString(TEXT("Black Static benchmark idle")))
			.ColorAndOpacity(FSlateColor(FLinearColor::White));

		SAssignNew(SummaryOverlay, SBorder)
			.Padding(FMargin(12.0f))
			[
				SummaryTextBlock.ToSharedRef()
			];

		GEngine->GameViewport->AddViewportWidgetContent(
			SNew(SWeakWidget).PossiblyNullContent(SummaryOverlay.ToSharedRef()),
			1000);
	}
}

void ABSBenchmarkRunner::BeginVisualRun(UBlackStaticBenchmarkScenarioData* InScenario, EBSBenchmarkMode Mode, bool bQueueFollowUpMode)
{
	Scenario = InScenario;
	check(Scenario);

	EnsureVisualActors();
	QueuedMode.Reset();
	if (bQueueFollowUpMode)
	{
		QueuedMode = EBSBenchmarkMode::Improved;
	}

	ActivePlanRequest = BuildVisualPlanRequest(Mode);
	ActivePlanResult = GetPlanner(Mode).Plan(ActivePlanRequest);
	LastMetrics = ExecuteBenchmark(Scenario, ActivePlanRequest, true);

	if (!BenchmarkPawn)
	{
		return;
	}

	bHasRecoveryReplanned = false;
	CurrentElapsedSeconds = 0.0f;
	TimeWithoutProgress = 0.0f;
	LastDistanceToGoal = TNumericLimits<float>::Max();
	StallMarkers.Reset();
	bIsRunning = ActivePlanResult.bSuccess;

	if (!ActivePlanResult.bSuccess)
	{
		FinishVisualRun(ActivePlanResult.FailureReason);
		return;
	}

	BenchmarkPawn->ResetRoute(ActivePlanResult.WorldPoints[0], ActivePlanResult.WorldPoints, ActivePlanRequest.AgentSpeedUnitsPerSecond, ActivePlanRequest.WaypointAcceptanceRadius);
	DrawScenarioDebug();
	UpdateOverlayText();
}

void ABSBenchmarkRunner::AdvanceVisualRun(float DeltaSeconds)
{
	if (!BenchmarkPawn || !BenchmarkPawn->HasRoute())
	{
		FinishVisualRun(TEXT("No active visual route."));
		return;
	}

	float DistanceAdvanced = 0.0f;
	const bool bReachedGoal = BenchmarkPawn->AdvanceAlongRoute(DeltaSeconds, DistanceAdvanced);
	CurrentElapsedSeconds += DeltaSeconds;

	const float DistanceToGoal = BenchmarkPawn->GetDistanceToGoal();
	if ((LastDistanceToGoal - DistanceToGoal) <= ActivePlanRequest.StallDistanceEpsilon)
	{
		TimeWithoutProgress += DeltaSeconds;
	}
	else
	{
		TimeWithoutProgress = 0.0f;
	}

	LastDistanceToGoal = DistanceToGoal;

	if (!bHasRecoveryReplanned && ActivePlanRequest.Mode == EBSBenchmarkMode::Improved && TimeWithoutProgress >= ActivePlanRequest.StallTimeoutSeconds)
	{
		TriggerRecoveryReplan();
		return;
	}

	if (bReachedGoal)
	{
		FinishVisualRun();
	}
}

void ABSBenchmarkRunner::TriggerRecoveryReplan()
{
	if (bHasRecoveryReplanned || !Scenario)
	{
		return;
	}

	bHasRecoveryReplanned = true;
	LastMetrics.StallEventsCount += 1;
	LastMetrics.ReplanCount = 1;
	StallMarkers.Add(BenchmarkPawn ? BenchmarkPawn->GetActorLocation() : GetActorLocation());

	ActivePlanRequest.ClearanceInflation += Scenario->RecoveryClearanceInflation;
	ActivePlanRequest.ClearanceWeight = Scenario->RecoveryClearanceWeight;
	ActivePlanResult = GetPlanner(EBSBenchmarkMode::Improved).Plan(ActivePlanRequest);

	if (!ActivePlanResult.bSuccess || !BenchmarkPawn)
	{
		FinishVisualRun(TEXT("Recovery replan failed."));
		return;
	}

	BenchmarkPawn->ResetRoute(ActivePlanResult.WorldPoints[0], ActivePlanResult.WorldPoints, ActivePlanRequest.AgentSpeedUnitsPerSecond, ActivePlanRequest.WaypointAcceptanceRadius);
	TimeWithoutProgress = 0.0f;
	LastDistanceToGoal = TNumericLimits<float>::Max();
	DrawScenarioDebug();
	UpdateOverlayText();
}

void ABSBenchmarkRunner::FinishVisualRun(const FString& FailureReason)
{
	bIsRunning = false;
	LastMetrics = ExecuteBenchmark(Scenario, ActivePlanRequest, false);
	LastMetrics.ReplanCount = FMath::Max(LastMetrics.ReplanCount, bHasRecoveryReplanned ? 1 : 0);
	LastMetrics.StallEventsCount = FMath::Max(LastMetrics.StallEventsCount, StallMarkers.Num());

	if (BenchmarkPawn)
	{
		LastMetrics.WorldPoints = BenchmarkPawn->GetExecutedPath();
		LastMetrics.ExecutedLength = CalculateWorldPathLength(LastMetrics.WorldPoints);
		if (ActivePlanRequest.AgentSpeedUnitsPerSecond > 0.0f)
		{
			LastMetrics.TimeToGoalSeconds = CurrentElapsedSeconds;
		}
	}

	if (!FailureReason.IsEmpty())
	{
		LastMetrics.bSuccess = false;
		LastMetrics.FailureReason = FailureReason;
	}

	CompletedVisualRuns.Add(LastMetrics);
	DrawScenarioDebug();
	UpdateOverlayText();

	if (QueuedMode.IsSet())
	{
		const EBSBenchmarkMode NextMode = QueuedMode.GetValue();
		QueuedMode.Reset();
		BeginVisualRun(Scenario, NextMode, false);
	}
	else if (PendingVisualRuns.Num() > 0)
	{
		PendingQueuedRunDelaySeconds = PauseBetweenQueuedRunsSeconds;
	}
	else
	{
		FlushVisualRunArtifacts();
	}
}

void ABSBenchmarkRunner::UpdateOverlayText()
{
	if (!SummaryTextBlock.IsValid())
	{
		return;
	}

	const FString Summary = FString::Printf(
		TEXT("Scenario: %s\nMode: %s\nSuccess: %s\nTime: %.2fs\nLength: %.2f\nPlanner: %.2fms\nTurns: %d\nReplans: %d\nCompleted Runs: %d\nQueued Runs: %d"),
		Scenario ? *Scenario->ScenarioId.ToString() : TEXT("None"),
		*LexToString(LastMetrics.Mode),
		LastMetrics.bSuccess ? TEXT("true") : TEXT("false"),
		LastMetrics.TimeToGoalSeconds,
		LastMetrics.ExecutedLength,
		LastMetrics.PlannerRuntimeMs,
		LastMetrics.TurnCount,
		LastMetrics.ReplanCount,
		CompletedVisualRuns.Num(),
		PendingVisualRuns.Num());

	SummaryTextBlock->SetText(FText::FromString(Summary));
}

void ABSBenchmarkRunner::DrawScenarioDebug() const
{
	if (!bDrawDebug || !GetWorld() || !Scenario)
	{
		return;
	}

	const FBSRuntimeGrid Grid = FBSGridUtils::BuildRuntimeGrid(ActivePlanRequest);

	for (const FBSGridRect& Area : Scenario->BlockedAreas)
	{
		FBSGridRect Normalized = Area;
		Normalized.Normalize();

		for (int32 X = Normalized.Min.X; X <= Normalized.Max.X; ++X)
		{
			for (int32 Y = Normalized.Min.Y; Y <= Normalized.Max.Y; ++Y)
			{
				const FVector Center = Grid.CellToWorld(FIntPoint(X, Y));
				DrawDebugSolidBox(GetWorld(), Center, FVector(Grid.CellSize * 0.45f, Grid.CellSize * 0.45f, 20.0f), FColor(32, 32, 32), false, DebugDrawDuration);
			}
		}
	}

	for (int32 Index = 1; Index < ActivePlanResult.WorldPoints.Num(); ++Index)
	{
		DrawDebugLine(GetWorld(), ActivePlanResult.WorldPoints[Index - 1], ActivePlanResult.WorldPoints[Index], GetModeColor(ActivePlanRequest.Mode), false, DebugDrawDuration, 0, 8.0f);
	}

	if (BenchmarkPawn)
	{
		const TArray<FVector>& ExecutedPath = BenchmarkPawn->GetExecutedPath();
		for (int32 Index = 1; Index < ExecutedPath.Num(); ++Index)
		{
			DrawDebugLine(GetWorld(), ExecutedPath[Index - 1], ExecutedPath[Index], FColor::White, false, DebugDrawDuration, 0, 4.0f);
		}
	}

	for (const FVector& Marker : StallMarkers)
	{
		DrawDebugSphere(GetWorld(), Marker, 30.0f, 12, FColor::Yellow, false, DebugDrawDuration, 0, 3.0f);
	}
}
