#include "Phase0/BlackStaticPhase0Statics.h"

#include "Phase0/Data/BSItemDefinition.h"

float UBSPhase0Statics::ComputeMovementNoiseLoudness(const float Speed, const float MaxWalkSpeed, const bool bIsCrouched, const bool bIsSprinting)
{
	if (Speed <= KINDA_SMALL_NUMBER || MaxWalkSpeed <= KINDA_SMALL_NUMBER)
	{
		return 0.0f;
	}

	float Loudness = FMath::Clamp(Speed / MaxWalkSpeed, 0.05f, 1.0f);
	if (bIsCrouched)
	{
		Loudness *= 0.45f;
	}

	if (bIsSprinting)
	{
		Loudness *= 1.25f;
	}

	return FMath::Clamp(Loudness, 0.0f, 1.5f);
}

int32 UBSPhase0Statics::ComputeScaledThreatCount(const int32 BaseThreatCount, const int32 PartySize)
{
	return FMath::Max(1, BaseThreatCount + FMath::Max(0, PartySize - 1));
}

int32 UBSPhase0Statics::ComputeUnlockTierFromReputation(const float Reputation)
{
	if (Reputation >= 125.0f)
	{
		return 3;
	}

	if (Reputation >= 60.0f)
	{
		return 2;
	}

	if (Reputation >= 20.0f)
	{
		return 1;
	}

	return 0;
}

FName UBSPhase0Statics::ResolveItemId(const FBSItemStack& Stack)
{
	if (!Stack.ItemId.IsNone())
	{
		return Stack.ItemId;
	}

	const TObjectPtr<UBSItemDefinition> LoadedDefinition = Stack.ItemDefinition.LoadSynchronous();
	return LoadedDefinition ? LoadedDefinition->ResolveItemId() : NAME_None;
}

FName UBSPhase0Statics::ResolveRequiredItemId(const FBSRequiredItem& Requirement)
{
	if (!Requirement.ItemId.IsNone())
	{
		return Requirement.ItemId;
	}

	const TObjectPtr<UBSItemDefinition> LoadedDefinition = Requirement.ItemDefinition.LoadSynchronous();
	return LoadedDefinition ? LoadedDefinition->ResolveItemId() : NAME_None;
}

bool UBSPhase0Statics::HasRequiredItems(const TArray<FBSItemStack>& Inventory, const TArray<FBSRequiredItem>& Requirements)
{
	for (const FBSRequiredItem& Requirement : Requirements)
	{
		if (GetQuantityForItem(Inventory, ResolveRequiredItemId(Requirement)) < Requirement.Quantity)
		{
			return false;
		}
	}

	return true;
}

void UBSPhase0Statics::ConsumeRequiredItems(TArray<FBSItemStack>& Inventory, const TArray<FBSRequiredItem>& Requirements)
{
	for (const FBSRequiredItem& Requirement : Requirements)
	{
		const FName ItemId = ResolveRequiredItemId(Requirement);
		int32 QuantityRemaining = Requirement.Quantity;

		for (int32 Index = Inventory.Num() - 1; Index >= 0 && QuantityRemaining > 0; --Index)
		{
			FBSItemStack& Stack = Inventory[Index];
			if (ResolveItemId(Stack) != ItemId)
			{
				continue;
			}

			const int32 QuantityToConsume = FMath::Min(Stack.Quantity, QuantityRemaining);
			Stack.Quantity -= QuantityToConsume;
			QuantityRemaining -= QuantityToConsume;

			if (Stack.Quantity <= 0)
			{
				Inventory.RemoveAt(Index);
			}
		}
	}
}

void UBSPhase0Statics::AddOrMergeItemStack(TArray<FBSItemStack>& Inventory, const FBSItemStack& Stack)
{
	if (!Stack.IsValid())
	{
		return;
	}

	const FName ResolvedItemId = ResolveItemId(Stack);
	if (ResolvedItemId.IsNone())
	{
		return;
	}

	for (FBSItemStack& Existing : Inventory)
	{
		if (ResolveItemId(Existing) == ResolvedItemId)
		{
			Existing.Quantity += Stack.Quantity;
			if (Existing.ItemDefinition.IsNull())
			{
				Existing.ItemDefinition = Stack.ItemDefinition;
			}
			if (Existing.ItemId.IsNone())
			{
				Existing.ItemId = ResolvedItemId;
			}
			return;
		}
	}

	FBSItemStack Copy = Stack;
	Copy.ItemId = ResolvedItemId;
	Inventory.Add(Copy);
}

int32 UBSPhase0Statics::GetQuantityForItem(const TArray<FBSItemStack>& Inventory, const FName ItemId)
{
	if (ItemId.IsNone())
	{
		return 0;
	}

	int32 Quantity = 0;
	for (const FBSItemStack& Stack : Inventory)
	{
		if (ResolveItemId(Stack) == ItemId)
		{
			Quantity += Stack.Quantity;
		}
	}

	return Quantity;
}
