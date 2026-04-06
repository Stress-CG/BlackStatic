#if WITH_DEV_AUTOMATION_TESTS

#include "Benchmark/BSAStarPlanner.h"
#include "Benchmark/BSGridUtils.h"
#include "Benchmark/BSThetaStarPlanner.h"
#include "Misc/AutomationTest.h"

namespace
{
	FBSPlanRequest MakeOpenRequest()
	{
		FBSPlanRequest Request;
		Request.ScenarioId = TEXT("AutomationOpen");
		Request.GridWidth = 8;
		Request.GridHeight = 8;
		Request.CellSize = 100.0f;
		Request.StartCell = FIntPoint(0, 0);
		Request.GoalCell = FIntPoint(5, 5);
		Request.bAllowDiagonal = true;
		return Request;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBlackStaticNeighborRulesTest, "BlackStatic.Benchmark.Neighbors", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FBlackStaticNeighborRulesTest::RunTest(const FString& Parameters)
{
	FBSPlanRequest Request;
	Request.GridWidth = 4;
	Request.GridHeight = 4;
	Request.StartCell = FIntPoint(0, 0);
	Request.GoalCell = FIntPoint(3, 3);
	Request.BlockedAreas =
	{
		{ FIntPoint(1, 0), FIntPoint(1, 0) },
		{ FIntPoint(0, 1), FIntPoint(0, 1) }
	};

	const FBSRuntimeGrid Grid = FBSGridUtils::BuildRuntimeGrid(Request);
	const TArray<FIntPoint> Neighbors = FBSGridUtils::GetNeighbors(Grid, FIntPoint(0, 0), true);
	TestFalse(TEXT("Diagonal corner cutting is blocked."), Neighbors.Contains(FIntPoint(1, 1)));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBlackStaticBlockedCellRejectionTest, "BlackStatic.Benchmark.BlockedCellRejection", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FBlackStaticBlockedCellRejectionTest::RunTest(const FString& Parameters)
{
	FBSAStarPlanner Planner;
	FBSPlanRequest Request = MakeOpenRequest();
	Request.BlockedAreas =
	{
		{ Request.StartCell, Request.StartCell }
	};

	const FBSPlanResult Result = Planner.Plan(Request);
	TestFalse(TEXT("Planning fails when the start cell is blocked."), Result.bSuccess);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBlackStaticAStarHeuristicTest, "BlackStatic.Benchmark.AStarHeuristic", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FBlackStaticAStarHeuristicTest::RunTest(const FString& Parameters)
{
	FBSAStarPlanner Planner;
	const FBSPlanRequest Request = MakeOpenRequest();
	const FBSPlanResult Result = Planner.Plan(Request);

	TestTrue(TEXT("A* succeeds on an open grid."), Result.bSuccess);
	TestEqual(TEXT("Open-grid A* path begins at the requested start cell."), Result.Cells[0], Request.StartCell);
	TestEqual(TEXT("Open-grid A* path ends at the requested goal cell."), Result.Cells.Last(), Request.GoalCell);
	TestTrue(TEXT("Planner cost stays close to the octile heuristic on an unobstructed grid."), FMath::IsNearlyEqual(Result.PlanCost, FBSGridUtils::OctileHeuristic(Request.StartCell, Request.GoalCell), 0.001));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBlackStaticThetaStarPruningTest, "BlackStatic.Benchmark.ThetaStarPruning", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FBlackStaticThetaStarPruningTest::RunTest(const FString& Parameters)
{
	FBSThetaStarPlanner Planner;
	FBSPlanRequest Request = MakeOpenRequest();
	Request.GoalCell = FIntPoint(5, 3);

	const FBSPlanResult Result = Planner.Plan(Request);
	TestTrue(TEXT("Theta* succeeds on an open grid."), Result.bSuccess);
	TestEqual(TEXT("Theta* prunes a straight line down to start and goal waypoints."), Result.Cells.Num(), 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBlackStaticDeterministicHashTest, "BlackStatic.Benchmark.DeterministicHash", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FBlackStaticDeterministicHashTest::RunTest(const FString& Parameters)
{
	FBSAStarPlanner Planner;
	FBSPlanRequest Request = MakeOpenRequest();
	Request.Seed = 20260406;

	const FBSPlanResult First = Planner.Plan(Request);
	const FBSPlanResult Second = Planner.Plan(Request);

	TestTrue(TEXT("Deterministic request produces a successful first plan."), First.bSuccess);
	TestEqual(TEXT("Deterministic request produces the same path hash."), First.PathHash, Second.PathHash);
	return true;
}

#endif

