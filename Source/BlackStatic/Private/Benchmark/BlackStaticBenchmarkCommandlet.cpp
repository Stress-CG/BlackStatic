#include "Benchmark/BlackStaticBenchmarkCommandlet.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Benchmark/BlackStaticBenchmarkRunner.h"
#include "Benchmark/BlackStaticBenchmarkScenarioData.h"
#include "HAL/FileManager.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "UObject/SavePackage.h"

#if WITH_EDITOR
#include "FileHelpers.h"
#endif

namespace
{
	struct FScenarioSeedDefinition
	{
		FName ScenarioId;
		FText DisplayName;
		bool bIncludeInSuite = true;
		int32 GridWidth = 32;
		int32 GridHeight = 18;
		FIntPoint StartCell = FIntPoint(1, 1);
		FIntPoint GoalCell = FIntPoint(10, 10);
		TArray<FBSGridRect> BlockedAreas;
		bool bUseTutorialRoadShowcaseMap = false;
	};

	FString GetScenarioPackageName(const FString& AssetName)
	{
		return FString::Printf(TEXT("/Game/Benchmarks/Scenarios/%s"), *AssetName);
	}

	FString GetScenarioAssetName(const FName& ScenarioId)
	{
		return FString::Printf(TEXT("DA_BS_%s"), *ScenarioId.ToString());
	}

	TArray<FScenarioSeedDefinition> BuildScenarioDefinitions()
	{
		TArray<FScenarioSeedDefinition> Definitions;

		Definitions.Add({
			TEXT("OpenLane"),
			FText::FromString(TEXT("Open Lane")),
			true,
			24,
			14,
			FIntPoint(2, 2),
			FIntPoint(21, 11),
			{}
		});

		Definitions.Add({
			TEXT("ZigZagCorridor"),
			FText::FromString(TEXT("Zig Zag Corridor")),
			true,
			30,
			18,
			FIntPoint(2, 2),
			FIntPoint(27, 15),
			{
				{ FIntPoint(6, 0), FIntPoint(7, 11) },
				{ FIntPoint(12, 6), FIntPoint(13, 17) },
				{ FIntPoint(18, 0), FIntPoint(19, 11) },
				{ FIntPoint(24, 6), FIntPoint(25, 17) }
			}
		});

		Definitions.Add({
			TEXT("NarrowGate"),
			FText::FromString(TEXT("Narrow Gate")),
			true,
			28,
			18,
			FIntPoint(2, 3),
			FIntPoint(25, 14),
			{
				{ FIntPoint(10, 0), FIntPoint(17, 6) },
				{ FIntPoint(10, 10), FIntPoint(17, 17) }
			}
		});

		Definitions.Add({
			TEXT("UTrap"),
			FText::FromString(TEXT("U Trap")),
			true,
			28,
			20,
			FIntPoint(3, 10),
			FIntPoint(23, 10),
			{
				{ FIntPoint(9, 4), FIntPoint(10, 15) },
				{ FIntPoint(10, 4), FIntPoint(18, 5) },
				{ FIntPoint(10, 14), FIntPoint(18, 15) }
			}
		});

		Definitions.Add({
			TEXT("TutorialRoadShowcase"),
			FText::FromString(TEXT("Tutorial Road Showcase")),
			false,
			36,
			20,
			FIntPoint(2, 10),
			FIntPoint(33, 10),
			{
				{ FIntPoint(8, 0), FIntPoint(10, 7) },
				{ FIntPoint(8, 13), FIntPoint(10, 19) },
				{ FIntPoint(16, 0), FIntPoint(18, 11) },
				{ FIntPoint(24, 8), FIntPoint(26, 19) }
			},
			true
		});

		return Definitions;
	}

	bool ParseModeList(const FString& RawMode, TArray<EBSBenchmarkMode>& OutModes)
	{
		const FString Normalized = RawMode.ToLower();
		if (Normalized == TEXT("baseline"))
		{
			OutModes.Add(EBSBenchmarkMode::Baseline);
			return true;
		}

		if (Normalized == TEXT("improved"))
		{
			OutModes.Add(EBSBenchmarkMode::Improved);
			return true;
		}

		if (Normalized == TEXT("all"))
		{
			OutModes.Add(EBSBenchmarkMode::Baseline);
			OutModes.Add(EBSBenchmarkMode::Improved);
			return true;
		}

		return false;
	}
}

UBlackStaticBenchmarkCommandlet::UBlackStaticBenchmarkCommandlet()
{
	IsClient = false;
	IsServer = false;
	IsEditor = true;
	LogToConsole = true;
	ShowErrorCount = true;
}

int32 UBlackStaticBenchmarkCommandlet::Main(const FString& Params)
{
	FString OutputDirectory;
	if (!FParse::Value(*Params, TEXT("OutputDir="), OutputDirectory))
	{
		OutputDirectory = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() / TEXT("Benchmarks/Commandlet"));
	}

	const bool bBootstrapAssets = FParse::Param(*Params, TEXT("BootstrapAssets"));
	const bool bBootstrapOnly = FParse::Param(*Params, TEXT("BootstrapOnly"));

	FString RawMode = TEXT("all");
	FParse::Value(*Params, TEXT("Mode="), RawMode);

	TArray<EBSBenchmarkMode> Modes;
	if (!ParseModeList(RawMode, Modes))
	{
		UE_LOG(LogTemp, Error, TEXT("Unsupported benchmark mode '%s'."), *RawMode);
		return 1;
	}

	if (bBootstrapAssets && !EnsureBenchmarkAssets())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to scaffold benchmark content."));
		return 1;
	}

	if (bBootstrapOnly)
	{
		return 0;
	}

	FAssetRegistryModule& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	FARFilter Filter;
	Filter.PackagePaths.Add(TEXT("/Game/Benchmarks/Scenarios"));
	Filter.ClassPaths.Add(UBlackStaticBenchmarkScenarioData::StaticClass()->GetClassPathName());
	Filter.bRecursivePaths = true;

	TArray<FAssetData> ScenarioAssets;
	AssetRegistry.Get().GetAssets(Filter, ScenarioAssets);
	if (ScenarioAssets.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("No benchmark scenario assets were found under /Game/Benchmarks/Scenarios."));
		return 1;
	}

	ScenarioAssets.Sort([](const FAssetData& Left, const FAssetData& Right)
	{
		return Left.AssetName.LexicalLess(Right.AssetName);
	});

	TArray<FBSBenchmarkMetrics> Runs;
	for (const FAssetData& AssetData : ScenarioAssets)
	{
		UBlackStaticBenchmarkScenarioData* Scenario = Cast<UBlackStaticBenchmarkScenarioData>(AssetData.GetAsset());
		if (!Scenario || !Scenario->bIncludeInCommandletSuite)
		{
			continue;
		}

		for (const EBSBenchmarkMode Mode : Modes)
		{
			const FBSBenchmarkMetrics Metrics = ABSBenchmarkRunner::RunSynchronousBenchmark(Scenario, Mode, OutputDirectory, true, true);
			Runs.Add(Metrics);
		}
	}

	if (Runs.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("No benchmark runs were produced."));
		return 1;
	}

	if (!ABSBenchmarkRunner::WriteAggregateSummary(Runs, OutputDirectory, nullptr))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to write aggregate benchmark summary."));
		return 1;
	}

	for (const FBSBenchmarkMetrics& Run : Runs)
	{
		if (!Run.bSuccess || Run.IllegalTraversalCount > 0)
		{
			return 1;
		}
	}

	return 0;
}

bool UBlackStaticBenchmarkCommandlet::EnsureBenchmarkAssets()
{
#if !WITH_EDITOR
	return false;
#else
	const FString BenchmarkMapPath = TEXT("/Game/Benchmarks/Maps/MAP_Benchmark");
	const FString TutorialRoadMapPath = TEXT("/Game/MAP_TutorialRoad_P");

	IFileManager::Get().MakeDirectory(*(FPaths::ProjectContentDir() / TEXT("Benchmarks/Scenarios")), true);
	IFileManager::Get().MakeDirectory(*(FPaths::ProjectContentDir() / TEXT("Benchmarks/Maps")), true);

	TArray<UBlackStaticBenchmarkScenarioData*> CreatedScenarios;
	for (const FScenarioSeedDefinition& Definition : BuildScenarioDefinitions())
	{
		const FString AssetName = GetScenarioAssetName(Definition.ScenarioId);
		const FString PackageName = GetScenarioPackageName(AssetName);
		UPackage* Package = CreatePackage(*PackageName);
		Package->FullyLoad();

		UBlackStaticBenchmarkScenarioData* Scenario = FindObject<UBlackStaticBenchmarkScenarioData>(Package, *AssetName);
		if (!Scenario)
		{
			Scenario = NewObject<UBlackStaticBenchmarkScenarioData>(Package, *AssetName, RF_Public | RF_Standalone);
			FAssetRegistryModule::AssetCreated(Scenario);
		}

		Scenario->ScenarioId = Definition.ScenarioId;
		Scenario->DisplayName = Definition.DisplayName;
		Scenario->bIncludeInCommandletSuite = Definition.bIncludeInSuite;
		Scenario->GridWidth = Definition.GridWidth;
		Scenario->GridHeight = Definition.GridHeight;
		Scenario->CellSize = 100.0f;
		Scenario->StartCell = Definition.StartCell;
		Scenario->GoalCell = Definition.GoalCell;
		Scenario->BlockedAreas = Definition.BlockedAreas;
		Scenario->Seed = 1337;
		Scenario->TurnPenaltySeconds = 0.18f;
		Scenario->ImprovedClearanceWeight = 0.15f;
		Scenario->RecoveryClearanceWeight = 0.80f;
		Scenario->RecoveryClearanceInflation = 1;
		Scenario->BenchmarkMap = TSoftObjectPtr<UWorld>(FSoftObjectPath(BenchmarkMapPath));
		Scenario->ShowcaseMap = Definition.bUseTutorialRoadShowcaseMap ? TSoftObjectPtr<UWorld>(FSoftObjectPath(TutorialRoadMapPath)) : TSoftObjectPtr<UWorld>();
		Scenario->MarkPackageDirty();

		const FString FilePath = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());
		FSavePackageArgs SaveArgs;
		SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
		SaveArgs.SaveFlags = SAVE_NoError;
		if (!UPackage::SavePackage(Package, Scenario, *FilePath, SaveArgs))
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to save scenario asset '%s'."), *PackageName);
			return false;
		}

		CreatedScenarios.Add(Scenario);
	}

	bool bMapExists = false;
	{
		FString ExistingFilename;
		bMapExists = FPackageName::DoesPackageExist(BenchmarkMapPath, &ExistingFilename);
	}

	const FString BenchmarkMapFilename = FPackageName::LongPackageNameToFilename(BenchmarkMapPath, FPackageName::GetMapPackageExtension());
	UWorld* BenchmarkWorld = nullptr;
	if (!bMapExists)
	{
		UWorld* NewWorld = UEditorLoadingAndSavingUtils::NewBlankMap(false);
		if (!NewWorld)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to create a blank benchmark map."));
			return false;
		}

		ABSBenchmarkRunner* Runner = NewWorld->SpawnActor<ABSBenchmarkRunner>(ABSBenchmarkRunner::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
		if (Runner && CreatedScenarios.Num() > 0)
		{
			Runner->Scenario = CreatedScenarios[0];
			Runner->bAutoRunOnBeginPlay = false;
			Runner->bRunBaselineAndImprovedBackToBack = true;
			Runner->AutoRunMode = EBSBenchmarkMode::Baseline;
			Runner->SetActorLabel(TEXT("BP_BenchmarkRunner"));
		}

		if (!UEditorLoadingAndSavingUtils::SaveMap(NewWorld, BenchmarkMapPath))
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to save benchmark map '%s'."), *BenchmarkMapPath);
			return false;
		}

		BenchmarkWorld = NewWorld;
	}
	else
	{
		BenchmarkWorld = UEditorLoadingAndSavingUtils::LoadMap(BenchmarkMapFilename);
	}

	if (!BenchmarkWorld)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load benchmark map for runner wiring."));
		return false;
	}

	TArray<AActor*> ExistingRunners;
	UGameplayStatics::GetAllActorsOfClass(BenchmarkWorld, ABSBenchmarkRunner::StaticClass(), ExistingRunners);
	ABSBenchmarkRunner* Runner = ExistingRunners.Num() > 0 ? Cast<ABSBenchmarkRunner>(ExistingRunners[0]) : nullptr;
	if (!Runner)
	{
		Runner = BenchmarkWorld->SpawnActor<ABSBenchmarkRunner>(ABSBenchmarkRunner::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
		if (Runner)
		{
			Runner->SetActorLabel(TEXT("BP_BenchmarkRunner"));
		}
	}

	if (!Runner)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create or locate the benchmark runner actor."));
		return false;
	}

	Runner->ScenarioSequence = CreatedScenarios;
	Runner->Scenario = CreatedScenarios.Num() > 0 ? CreatedScenarios[0] : nullptr;
	Runner->bAutoRunOnBeginPlay = true;
	Runner->bCycleScenarioSequence = true;
	Runner->bDiscoverScenariosWhenSequenceEmpty = true;
	Runner->bRunBaselineAndImprovedBackToBack = true;
	Runner->PauseBetweenQueuedRunsSeconds = 1.0f;
	Runner->bWriteArtifactsAfterVisualRuns = true;
	Runner->VisualRunOutputDirectory = TEXT("Saved/DemoRuns");
	Runner->AutoRunMode = EBSBenchmarkMode::Baseline;
	Runner->Modify();

	if (!UEditorLoadingAndSavingUtils::SaveMap(BenchmarkWorld, BenchmarkMapPath))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to persist benchmark map runner wiring."));
		return false;
	}

	return true;
#endif
}
