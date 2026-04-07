#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "BSPhase0BootstrapCommandlet.generated.h"

UCLASS()
class BLACKSTATIC_API UBSPhase0BootstrapCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UBSPhase0BootstrapCommandlet();

	virtual int32 Main(const FString& Params) override;
};
