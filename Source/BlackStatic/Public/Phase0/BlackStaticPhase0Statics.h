#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Phase0/BlackStaticPhase0Types.h"
#include "BlackStaticPhase0Statics.generated.h"

class UBSItemDefinition;

UCLASS()
class BLACKSTATIC_API UBSPhase0Statics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Black Static|Phase0")
	static float ComputeMovementNoiseLoudness(float Speed, float MaxWalkSpeed, bool bIsCrouched, bool bIsSprinting);

	UFUNCTION(BlueprintPure, Category = "Black Static|Phase0")
	static int32 ComputeScaledThreatCount(int32 BaseThreatCount, int32 PartySize);

	UFUNCTION(BlueprintPure, Category = "Black Static|Phase0")
	static int32 ComputeUnlockTierFromReputation(float Reputation);

	UFUNCTION(BlueprintPure, Category = "Black Static|Phase0")
	static FName ResolveItemId(const FBSItemStack& Stack);

	UFUNCTION(BlueprintPure, Category = "Black Static|Phase0")
	static FName ResolveRequiredItemId(const FBSRequiredItem& Requirement);

	static bool HasRequiredItems(const TArray<FBSItemStack>& Inventory, const TArray<FBSRequiredItem>& Requirements);
	static void ConsumeRequiredItems(TArray<FBSItemStack>& Inventory, const TArray<FBSRequiredItem>& Requirements);
	static void AddOrMergeItemStack(TArray<FBSItemStack>& Inventory, const FBSItemStack& Stack);
	static int32 GetQuantityForItem(const TArray<FBSItemStack>& Inventory, FName ItemId);
};
