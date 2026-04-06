#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "BlackStaticBenchmarkCommandlet.generated.h"

UCLASS()
class BLACKSTATIC_API UBlackStaticBenchmarkCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UBlackStaticBenchmarkCommandlet();

	virtual int32 Main(const FString& Params) override;

private:
	bool EnsureBenchmarkAssets();
};

