#pragma once

#include "CoreMinimal.h"
#include "Benchmark/BSTypes.h"

struct FBSRuntimeGrid
{
	int32 Width = 0;
	int32 Height = 0;
	float CellSize = 100.0f;
	FVector WorldOrigin = FVector::ZeroVector;
	TSet<int64> BlockedCells;
	TMap<int64, float> ClearanceByCell;

	bool IsInBounds(const FIntPoint& Cell) const;
	bool IsBlocked(const FIntPoint& Cell) const;
	float GetClearance(const FIntPoint& Cell) const;
	FVector CellToWorld(const FIntPoint& Cell) const;
};

class BLACKSTATIC_API FBSGridUtils
{
public:
	static int64 MakeCellKey(const FIntPoint& Cell);
	static FBSRuntimeGrid BuildRuntimeGrid(const FBSPlanRequest& Request);
	static TArray<FIntPoint> GetNeighbors(const FBSRuntimeGrid& Grid, const FIntPoint& Cell, bool bAllowDiagonal);
	static bool HasLineOfSight(const FBSRuntimeGrid& Grid, const FIntPoint& StartCell, const FIntPoint& EndCell);
	static double OctileHeuristic(const FIntPoint& FromCell, const FIntPoint& ToCell);
	static double GetMoveCost(const FIntPoint& FromCell, const FIntPoint& ToCell);
	static double GetClearancePenalty(const FBSRuntimeGrid& Grid, const FIntPoint& Cell, float ClearanceWeight);
	static TArray<FIntPoint> SimplifyPath(const FBSRuntimeGrid& Grid, const TArray<FIntPoint>& Path);
	static float CalculatePathLengthWorld(const FBSRuntimeGrid& Grid, const TArray<FIntPoint>& Path);
	static int32 CountDirectionChanges(const TArray<FIntPoint>& Path);
	static int32 CountIllegalTraversals(const FBSRuntimeGrid& Grid, const TArray<FIntPoint>& Path);
	static float CalculateMinimumClearanceWorld(const FBSRuntimeGrid& Grid, const TArray<FIntPoint>& Path);
	static FString HashPath(const TArray<FIntPoint>& Path);
};

