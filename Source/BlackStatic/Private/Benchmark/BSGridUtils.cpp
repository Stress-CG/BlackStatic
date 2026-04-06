#include "Benchmark/BSGridUtils.h"

#include "Algo/Reverse.h"
#include "Containers/Array.h"
#include "Math/UnrealMathUtility.h"
#include "Misc/SecureHash.h"

namespace
{
	constexpr int32 CardinalDirections[4][2] =
	{
		{ 0, 1 },
		{ 1, 0 },
		{ 0, -1 },
		{ -1, 0 }
	};

	constexpr int32 DiagonalDirections[4][2] =
	{
		{ 1, 1 },
		{ 1, -1 },
		{ -1, 1 },
		{ -1, -1 }
	};
}

FString LexToString(EBSBenchmarkMode Mode)
{
	switch (Mode)
	{
	case EBSBenchmarkMode::Baseline:
		return TEXT("baseline");
	case EBSBenchmarkMode::Improved:
		return TEXT("improved");
	default:
		return TEXT("unknown");
	}
}

bool FBSRuntimeGrid::IsInBounds(const FIntPoint& Cell) const
{
	return Cell.X >= 0 && Cell.Y >= 0 && Cell.X < Width && Cell.Y < Height;
}

bool FBSRuntimeGrid::IsBlocked(const FIntPoint& Cell) const
{
	return !IsInBounds(Cell) || BlockedCells.Contains(FBSGridUtils::MakeCellKey(Cell));
}

float FBSRuntimeGrid::GetClearance(const FIntPoint& Cell) const
{
	if (!IsInBounds(Cell))
	{
		return 0.0f;
	}

	if (const float* FoundClearance = ClearanceByCell.Find(FBSGridUtils::MakeCellKey(Cell)))
	{
		return *FoundClearance;
	}

	return 0.0f;
}

FVector FBSRuntimeGrid::CellToWorld(const FIntPoint& Cell) const
{
	return WorldOrigin + FVector((static_cast<float>(Cell.X) + 0.5f) * CellSize, (static_cast<float>(Cell.Y) + 0.5f) * CellSize, 0.0f);
}

int64 FBSGridUtils::MakeCellKey(const FIntPoint& Cell)
{
	return (static_cast<int64>(Cell.X) << 32) ^ static_cast<uint32>(Cell.Y);
}

FBSRuntimeGrid FBSGridUtils::BuildRuntimeGrid(const FBSPlanRequest& Request)
{
	FBSRuntimeGrid Grid;
	Grid.Width = Request.GridWidth;
	Grid.Height = Request.GridHeight;
	Grid.CellSize = Request.CellSize;
	Grid.WorldOrigin = Request.WorldOrigin;

	for (const FBSGridRect& RawArea : Request.BlockedAreas)
	{
		FBSGridRect Area = RawArea;
		Area.Normalize();

		const int32 MinX = FMath::Clamp(Area.Min.X - Request.ClearanceInflation, 0, Request.GridWidth - 1);
		const int32 MaxX = FMath::Clamp(Area.Max.X + Request.ClearanceInflation, 0, Request.GridWidth - 1);
		const int32 MinY = FMath::Clamp(Area.Min.Y - Request.ClearanceInflation, 0, Request.GridHeight - 1);
		const int32 MaxY = FMath::Clamp(Area.Max.Y + Request.ClearanceInflation, 0, Request.GridHeight - 1);

		for (int32 X = MinX; X <= MaxX; ++X)
		{
			for (int32 Y = MinY; Y <= MaxY; ++Y)
			{
				Grid.BlockedCells.Add(MakeCellKey(FIntPoint(X, Y)));
			}
		}
	}

	TArray<FIntPoint> BlockedCellList;
	BlockedCellList.Reserve(Grid.BlockedCells.Num());
	for (const int64 CellKey : Grid.BlockedCells)
	{
		const int32 X = static_cast<int32>(CellKey >> 32);
		const int32 Y = static_cast<int32>(CellKey & 0xffffffff);
		BlockedCellList.Add(FIntPoint(X, Y));
	}

	for (int32 X = 0; X < Grid.Width; ++X)
	{
		for (int32 Y = 0; Y < Grid.Height; ++Y)
		{
			const FIntPoint Cell(X, Y);
			const int64 CellKey = MakeCellKey(Cell);
			if (Grid.BlockedCells.Contains(CellKey))
			{
				Grid.ClearanceByCell.Add(CellKey, 0.0f);
				continue;
			}

			float MinDistance = static_cast<float>(FMath::Max(Grid.Width, Grid.Height));
			for (const FIntPoint& BlockedCell : BlockedCellList)
			{
				const float CellDistance = FVector2D::Distance(FVector2D(Cell), FVector2D(BlockedCell));
				MinDistance = FMath::Min(MinDistance, CellDistance);
			}

			Grid.ClearanceByCell.Add(CellKey, BlockedCellList.Num() > 0 ? MinDistance : static_cast<float>(FMath::Max(Grid.Width, Grid.Height)));
		}
	}

	return Grid;
}

TArray<FIntPoint> FBSGridUtils::GetNeighbors(const FBSRuntimeGrid& Grid, const FIntPoint& Cell, bool bAllowDiagonal)
{
	TArray<FIntPoint> Neighbors;
	Neighbors.Reserve(8);

	for (const int32 (&Direction)[2] : CardinalDirections)
	{
		const FIntPoint Neighbor(Cell.X + Direction[0], Cell.Y + Direction[1]);
		if (!Grid.IsBlocked(Neighbor))
		{
			Neighbors.Add(Neighbor);
		}
	}

	if (!bAllowDiagonal)
	{
		return Neighbors;
	}

	for (const int32 (&Direction)[2] : DiagonalDirections)
	{
		const FIntPoint Neighbor(Cell.X + Direction[0], Cell.Y + Direction[1]);
		if (Grid.IsBlocked(Neighbor))
		{
			continue;
		}

		const FIntPoint Horizontal(Cell.X + Direction[0], Cell.Y);
		const FIntPoint Vertical(Cell.X, Cell.Y + Direction[1]);
		if (Grid.IsBlocked(Horizontal) || Grid.IsBlocked(Vertical))
		{
			continue;
		}

		Neighbors.Add(Neighbor);
	}

	return Neighbors;
}

bool FBSGridUtils::HasLineOfSight(const FBSRuntimeGrid& Grid, const FIntPoint& StartCell, const FIntPoint& EndCell)
{
	const FVector2D Start(static_cast<float>(StartCell.X) + 0.5f, static_cast<float>(StartCell.Y) + 0.5f);
	const FVector2D End(static_cast<float>(EndCell.X) + 0.5f, static_cast<float>(EndCell.Y) + 0.5f);
	const float DeltaX = End.X - Start.X;
	const float DeltaY = End.Y - Start.Y;
	const int32 Samples = FMath::Max(FMath::Abs(EndCell.X - StartCell.X), FMath::Abs(EndCell.Y - StartCell.Y)) * 8 + 1;

	for (int32 SampleIndex = 0; SampleIndex <= Samples; ++SampleIndex)
	{
		const float Alpha = Samples == 0 ? 0.0f : static_cast<float>(SampleIndex) / static_cast<float>(Samples);
		const FVector2D Point = FMath::Lerp(Start, End, Alpha);
		const FIntPoint SampleCell(FMath::FloorToInt(Point.X), FMath::FloorToInt(Point.Y));
		if (Grid.IsBlocked(SampleCell))
		{
			return false;
		}
	}

	return true;
}

double FBSGridUtils::OctileHeuristic(const FIntPoint& FromCell, const FIntPoint& ToCell)
{
	const double DeltaX = FMath::Abs(FromCell.X - ToCell.X);
	const double DeltaY = FMath::Abs(FromCell.Y - ToCell.Y);
	const double Diagonal = FMath::Min(DeltaX, DeltaY);
	const double Straight = DeltaX + DeltaY - (2.0 * Diagonal);
	return (Diagonal * UE_SQRT_2) + Straight;
}

double FBSGridUtils::GetMoveCost(const FIntPoint& FromCell, const FIntPoint& ToCell)
{
	return FVector2D::Distance(FVector2D(FromCell), FVector2D(ToCell));
}

double FBSGridUtils::GetClearancePenalty(const FBSRuntimeGrid& Grid, const FIntPoint& Cell, float ClearanceWeight)
{
	if (ClearanceWeight <= KINDA_SMALL_NUMBER)
	{
		return 0.0;
	}

	const float Clearance = Grid.GetClearance(Cell);
	const float SafeClearance = FMath::Max(Clearance, 1.0f);
	return ClearanceWeight / static_cast<double>(SafeClearance);
}

TArray<FIntPoint> FBSGridUtils::SimplifyPath(const FBSRuntimeGrid& Grid, const TArray<FIntPoint>& Path)
{
	if (Path.Num() <= 2)
	{
		return Path;
	}

	TArray<FIntPoint> Simplified;
	Simplified.Add(Path[0]);

	int32 AnchorIndex = 0;
	int32 ProbeIndex = 2;
	while (ProbeIndex < Path.Num())
	{
		if (!HasLineOfSight(Grid, Path[AnchorIndex], Path[ProbeIndex]))
		{
			Simplified.Add(Path[ProbeIndex - 1]);
			AnchorIndex = ProbeIndex - 1;
		}

		++ProbeIndex;
	}

	Simplified.Add(Path.Last());
	return Simplified;
}

float FBSGridUtils::CalculatePathLengthWorld(const FBSRuntimeGrid& Grid, const TArray<FIntPoint>& Path)
{
	if (Path.Num() < 2)
	{
		return 0.0f;
	}

	float TotalLength = 0.0f;
	for (int32 Index = 1; Index < Path.Num(); ++Index)
	{
		TotalLength += FVector::Distance(Grid.CellToWorld(Path[Index - 1]), Grid.CellToWorld(Path[Index]));
	}

	return TotalLength;
}

int32 FBSGridUtils::CountDirectionChanges(const TArray<FIntPoint>& Path)
{
	if (Path.Num() < 3)
	{
		return 0;
	}

	int32 TurnCount = 0;
	auto ToSign = [](const FIntPoint& Value)
	{
		return FIntPoint(FMath::Sign(Value.X), FMath::Sign(Value.Y));
	};

	FIntPoint PreviousDirection = ToSign(Path[1] - Path[0]);
	for (int32 Index = 2; Index < Path.Num(); ++Index)
	{
		const FIntPoint CurrentDirection = ToSign(Path[Index] - Path[Index - 1]);
		if (CurrentDirection != PreviousDirection)
		{
			++TurnCount;
		}

		PreviousDirection = CurrentDirection;
	}

	return TurnCount;
}

int32 FBSGridUtils::CountIllegalTraversals(const FBSRuntimeGrid& Grid, const TArray<FIntPoint>& Path)
{
	int32 IllegalSamples = 0;

	for (int32 SegmentIndex = 1; SegmentIndex < Path.Num(); ++SegmentIndex)
	{
		const FIntPoint SegmentStart = Path[SegmentIndex - 1];
		const FIntPoint SegmentEnd = Path[SegmentIndex];
		const FVector2D Start(static_cast<float>(SegmentStart.X) + 0.5f, static_cast<float>(SegmentStart.Y) + 0.5f);
		const FVector2D End(static_cast<float>(SegmentEnd.X) + 0.5f, static_cast<float>(SegmentEnd.Y) + 0.5f);
		const int32 Samples = FMath::Max(FMath::Abs(SegmentEnd.X - SegmentStart.X), FMath::Abs(SegmentEnd.Y - SegmentStart.Y)) * 8 + 1;

		for (int32 SampleIndex = 0; SampleIndex <= Samples; ++SampleIndex)
		{
			const float Alpha = Samples == 0 ? 0.0f : static_cast<float>(SampleIndex) / static_cast<float>(Samples);
			const FVector2D Point = FMath::Lerp(Start, End, Alpha);
			const FIntPoint SampleCell(FMath::FloorToInt(Point.X), FMath::FloorToInt(Point.Y));
			if (Grid.IsBlocked(SampleCell))
			{
				++IllegalSamples;
			}
		}
	}

	return IllegalSamples;
}

float FBSGridUtils::CalculateMinimumClearanceWorld(const FBSRuntimeGrid& Grid, const TArray<FIntPoint>& Path)
{
	if (Path.Num() == 0)
	{
		return 0.0f;
	}

	float MinClearance = TNumericLimits<float>::Max();
	for (const FIntPoint& Cell : Path)
	{
		MinClearance = FMath::Min(MinClearance, Grid.GetClearance(Cell) * Grid.CellSize);
	}

	return MinClearance == TNumericLimits<float>::Max() ? 0.0f : MinClearance;
}

FString FBSGridUtils::HashPath(const TArray<FIntPoint>& Path)
{
	FString Buffer;
	for (const FIntPoint& Cell : Path)
	{
		Buffer += FString::Printf(TEXT("%d:%d;"), Cell.X, Cell.Y);
	}

	FMD5 Md5;
	FTCHARToUTF8 Utf8(*Buffer);
	Md5.Update(reinterpret_cast<const uint8*>(Utf8.Get()), Utf8.Length());
	uint8 Digest[16];
	Md5.Final(Digest);
	return BytesToHex(Digest, UE_ARRAY_COUNT(Digest));
}
