#pragma once

#include "CoreMinimal.h"
#include "Benchmark/IBSPathPlanner.h"

class BLACKSTATIC_API FBSThetaStarPlanner : public IBSPathPlanner
{
public:
	virtual FName GetPlannerId() const override;
	virtual FBSPlanResult Plan(const FBSPlanRequest& Request) const override;
};

