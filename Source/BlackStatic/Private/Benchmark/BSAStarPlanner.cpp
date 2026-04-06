#include "Benchmark/BSAStarPlanner.h"

#include "Algo/Reverse.h"
#include "Benchmark/BSGridUtils.h"

namespace
{
	struct FBSOpenNode
	{
		FIntPoint Cell = FIntPoint::ZeroValue;
		double FScore = 0.0;
		double HScore = 0.0;
		int32 Order = 0;
	};

	TArray<FIntPoint> ReconstructPath(const TMap<int64, FIntPoint>& ParentMap, const FIntPoint& StartCell, const FIntPoint& GoalCell)
	{
		TArray<FIntPoint> Result;
		Result.Add(GoalCell);

		FIntPoint Current = GoalCell;
		while (Current != StartCell)
		{
			const FIntPoint* Parent = ParentMap.Find(FBSGridUtils::MakeCellKey(Current));
			if (!Parent)
			{
				return {};
			}

			Current = *Parent;
			Result.Add(Current);
		}

		Algo::Reverse(Result);
		return Result;
	}
}

FName FBSAStarPlanner::GetPlannerId() const
{
	return TEXT("AStar");
}

FBSPlanResult FBSAStarPlanner::Plan(const FBSPlanRequest& Request) const
{
	const double StartTime = FPlatformTime::Seconds();
	FBSPlanResult Result;
	Result.Mode = Request.Mode;
	Result.PlannerLabel = Request.PlannerLabel;

	const FBSRuntimeGrid Grid = FBSGridUtils::BuildRuntimeGrid(Request);
	if (Grid.IsBlocked(Request.StartCell) || Grid.IsBlocked(Request.GoalCell))
	{
		Result.FailureReason = TEXT("Start or goal cell is blocked.");
		Result.PlannerRuntimeMs = (FPlatformTime::Seconds() - StartTime) * 1000.0;
		return Result;
	}

	TArray<FBSOpenNode> OpenSet;
	TSet<int64> ClosedSet;
	TMap<int64, double> GScore;
	TMap<int64, FIntPoint> ParentMap;
	int32 InsertionOrder = 0;

	const int64 StartKey = FBSGridUtils::MakeCellKey(Request.StartCell);
	GScore.Add(StartKey, 0.0);
	OpenSet.Add({ Request.StartCell, FBSGridUtils::OctileHeuristic(Request.StartCell, Request.GoalCell), 0.0, InsertionOrder++ });

	while (OpenSet.Num() > 0)
	{
		int32 BestIndex = 0;
		for (int32 CandidateIndex = 1; CandidateIndex < OpenSet.Num(); ++CandidateIndex)
		{
			const FBSOpenNode& Candidate = OpenSet[CandidateIndex];
			const FBSOpenNode& BestNode = OpenSet[BestIndex];

			if (Candidate.FScore < BestNode.FScore ||
				(FMath::IsNearlyEqual(Candidate.FScore, BestNode.FScore) && Candidate.HScore < BestNode.HScore) ||
				(FMath::IsNearlyEqual(Candidate.FScore, BestNode.FScore) && FMath::IsNearlyEqual(Candidate.HScore, BestNode.HScore) && Candidate.Order < BestNode.Order))
			{
				BestIndex = CandidateIndex;
			}
		}

		const FBSOpenNode CurrentNode = OpenSet[BestIndex];
		OpenSet.RemoveAtSwap(BestIndex, 1, EAllowShrinking::No);

		const int64 CurrentKey = FBSGridUtils::MakeCellKey(CurrentNode.Cell);
		if (ClosedSet.Contains(CurrentKey))
		{
			continue;
		}

		ClosedSet.Add(CurrentKey);
		++Result.ExpandedNodes;

		if (CurrentNode.Cell == Request.GoalCell)
		{
			Result.bSuccess = true;
			Result.Cells = ReconstructPath(ParentMap, Request.StartCell, Request.GoalCell);
			Result.PathLengthUnits = FBSGridUtils::CalculatePathLengthWorld(Grid, Result.Cells);
			Result.MinimumClearanceUnits = FBSGridUtils::CalculateMinimumClearanceWorld(Grid, Result.Cells);
			Result.PathHash = FBSGridUtils::HashPath(Result.Cells);
			for (const FIntPoint& Cell : Result.Cells)
			{
				Result.WorldPoints.Add(Grid.CellToWorld(Cell));
			}

			if (const double* GoalScore = GScore.Find(CurrentKey))
			{
				Result.PlanCost = *GoalScore;
			}

			Result.PlannerRuntimeMs = (FPlatformTime::Seconds() - StartTime) * 1000.0;
			return Result;
		}

		const TArray<FIntPoint> Neighbors = FBSGridUtils::GetNeighbors(Grid, CurrentNode.Cell, Request.bAllowDiagonal);
		for (const FIntPoint& Neighbor : Neighbors)
		{
			const int64 NeighborKey = FBSGridUtils::MakeCellKey(Neighbor);
			if (ClosedSet.Contains(NeighborKey))
			{
				continue;
			}

			const double CurrentScore = GScore.FindRef(CurrentKey);
			const double TentativeScore = CurrentScore + FBSGridUtils::GetMoveCost(CurrentNode.Cell, Neighbor) + FBSGridUtils::GetClearancePenalty(Grid, Neighbor, Request.ClearanceWeight);
			const double ExistingScore = GScore.Contains(NeighborKey) ? GScore.FindRef(NeighborKey) : TNumericLimits<double>::Max();
			if (TentativeScore >= ExistingScore)
			{
				continue;
			}

			ParentMap.Add(NeighborKey, CurrentNode.Cell);
			GScore.Add(NeighborKey, TentativeScore);
			const double Heuristic = FBSGridUtils::OctileHeuristic(Neighbor, Request.GoalCell);
			OpenSet.Add({ Neighbor, TentativeScore + Heuristic, Heuristic, InsertionOrder++ });
		}
	}

	Result.FailureReason = TEXT("No path found.");
	Result.PlannerRuntimeMs = (FPlatformTime::Seconds() - StartTime) * 1000.0;
	return Result;
}

