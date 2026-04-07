#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Phase0/BlackStaticPhase0Types.h"
#include "BSSaveGame.generated.h"

UCLASS()
class BLACKSTATIC_API UBSSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Settlement")
	FBSSettlementState SettlementState;
};
