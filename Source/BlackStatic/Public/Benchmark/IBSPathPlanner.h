#pragma once

#include "CoreMinimal.h"
#include "Benchmark/BSTypes.h"

class BLACKSTATIC_API IBSPathPlanner
{
public:
	virtual ~IBSPathPlanner() = default;

	virtual FName GetPlannerId() const = 0;
	virtual FBSPlanResult Plan(const FBSPlanRequest& Request) const = 0;
};

