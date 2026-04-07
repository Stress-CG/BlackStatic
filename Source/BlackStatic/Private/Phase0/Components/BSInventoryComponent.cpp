#include "Phase0/Components/BSInventoryComponent.h"

#include "Phase0/BlackStaticPhase0Statics.h"
#include "Phase0/Data/BSItemDefinition.h"

UBSInventoryComponent::UBSInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UBSInventoryComponent::AddItemDefinition(UBSItemDefinition* ItemDefinition, const int32 Quantity)
{
	if (!ItemDefinition || Quantity <= 0)
	{
		return false;
	}

	FBSItemStack Stack;
	Stack.ItemDefinition = ItemDefinition;
	Stack.ItemId = ItemDefinition->ResolveItemId();
	Stack.Quantity = Quantity;
	return AddItemStack(Stack);
}

bool UBSInventoryComponent::AddItemStack(const FBSItemStack& Stack)
{
	if (!Stack.IsValid())
	{
		return false;
	}

	UBSPhase0Statics::AddOrMergeItemStack(Items, Stack);
	OnInventoryChanged.Broadcast();
	return true;
}

bool UBSInventoryComponent::RemoveItemById(const FName ItemId, const int32 Quantity)
{
	if (ItemId.IsNone() || Quantity <= 0 || GetQuantityById(ItemId) < Quantity)
	{
		return false;
	}

	int32 Remaining = Quantity;
	for (int32 Index = Items.Num() - 1; Index >= 0 && Remaining > 0; --Index)
	{
		FBSItemStack& Stack = Items[Index];
		if (UBSPhase0Statics::ResolveItemId(Stack) != ItemId)
		{
			continue;
		}

		const int32 QuantityToRemove = FMath::Min(Stack.Quantity, Remaining);
		Stack.Quantity -= QuantityToRemove;
		Remaining -= QuantityToRemove;
		if (Stack.Quantity <= 0)
		{
			Items.RemoveAt(Index);
		}
	}

	OnInventoryChanged.Broadcast();
	return true;
}

bool UBSInventoryComponent::ConsumeRequiredItems(const TArray<FBSRequiredItem>& Requirements)
{
	if (!HasRequiredItems(Requirements))
	{
		return false;
	}

	UBSPhase0Statics::ConsumeRequiredItems(Items, Requirements);
	OnInventoryChanged.Broadcast();
	return true;
}

bool UBSInventoryComponent::HasRequiredItems(const TArray<FBSRequiredItem>& Requirements) const
{
	return UBSPhase0Statics::HasRequiredItems(Items, Requirements);
}

int32 UBSInventoryComponent::GetQuantityById(const FName ItemId) const
{
	return UBSPhase0Statics::GetQuantityForItem(Items, ItemId);
}

const TArray<FBSItemStack>& UBSInventoryComponent::GetItems() const
{
	return Items;
}

void UBSInventoryComponent::ClearInventory()
{
	Items.Reset();
	OnInventoryChanged.Broadcast();
}

void UBSInventoryComponent::TransferAllItemsTo(UBSInventoryComponent* OtherInventory)
{
	if (!OtherInventory || OtherInventory == this)
	{
		return;
	}

	for (const FBSItemStack& Stack : Items)
	{
		OtherInventory->AddItemStack(Stack);
	}

	ClearInventory();
}

void UBSInventoryComponent::SetInventoryItems(const TArray<FBSItemStack>& NewItems)
{
	Items = NewItems;
	OnInventoryChanged.Broadcast();
}
