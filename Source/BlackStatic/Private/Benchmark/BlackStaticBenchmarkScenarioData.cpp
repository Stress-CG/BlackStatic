#include "Benchmark/BlackStaticBenchmarkScenarioData.h"

FBSPlanRequest UBlackStaticBenchmarkScenarioData::MakePlanRequest(EBSBenchmarkMode Mode, const FVector& AdditionalWorldOffset) const
{
	FBSPlanRequest Request;
	Request.ScenarioId = ScenarioId.ToString();
	Request.Mode = Mode;
	Request.PlannerLabel = Mode == EBSBenchmarkMode::Baseline ? TEXT("A* Baseline") : TEXT("Theta* Improved");
	Request.GridWidth = GridWidth;
	Request.GridHeight = GridHeight;
	Request.CellSize = CellSize;
	Request.WorldOrigin = LocalWorldOrigin + AdditionalWorldOffset;
	Request.StartCell = StartCell;
	Request.GoalCell = GoalCell;
	Request.BlockedAreas = BlockedAreas;
	Request.Seed = Seed;
	Request.AgentSpeedUnitsPerSecond = Mode == EBSBenchmarkMode::Baseline ? BaselineAgentSpeed : ImprovedAgentSpeed;
	Request.TurnPenaltySeconds = TurnPenaltySeconds;
	Request.WaypointAcceptanceRadius = WaypointAcceptanceRadius;
	Request.StallDistanceEpsilon = StallDistanceEpsilon;
	Request.StallTimeoutSeconds = StallTimeoutSeconds;
	Request.MaxReplans = Mode == EBSBenchmarkMode::Improved ? 1 : 0;
	Request.ClearanceWeight = Mode == EBSBenchmarkMode::Improved ? ImprovedClearanceWeight : 0.0f;
	Request.ClearanceInflation = 0;
	return Request;
}

