#pragma once

#include "CoreMinimal.h"
#include "Engine/PrimaryDataAsset.h"
#include "Phase0/BlackStaticPhase0Types.h"
#include "BSTaskDefinition.generated.h"

UCLASS(BlueprintType)
class BLACKSTATIC_API UBSTaskDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Task")
	FName TaskId = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Task")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Task", meta = (MultiLine = "true"))
	FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Task")
	EBSObjectiveType ObjectiveType = EBSObjectiveType::WaterPowerRestoration;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Task")
	TArray<FBSRequiredItem> RequiredItems;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Task")
	FBSTaskReward Reward;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Task")
	bool bRepeatable = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Task", meta = (ClampMin = "1"))
	int32 BaseThreatCount = 3;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Task")
	bool bRequireExtractionAfterSiteCompletion = true;

	UFUNCTION(BlueprintPure, Category = "Task")
	FName ResolveTaskId() const;
};
