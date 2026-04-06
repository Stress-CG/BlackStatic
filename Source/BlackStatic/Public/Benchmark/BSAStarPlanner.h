#pragma once

#include "CoreMinimal.h"
#include "Benchmark/IBSPathPlanner.h"

class BLACKSTATIC_API FBSAStarPlanner : public IBSPathPlanner
{
public:
	virtual FName GetPlannerId() const override;
	virtual FBSPlanResult Plan(const FBSPlanRequest& Request) const override;
};

