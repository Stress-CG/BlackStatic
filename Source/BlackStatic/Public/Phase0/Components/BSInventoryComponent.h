#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Phase0/BlackStaticPhase0Types.h"
#include "BSInventoryComponent.generated.h"

class UBSItemDefinition;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FBSInventoryChangedSignature);

UCLASS(ClassGroup = (BlackStatic), BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class BLACKSTATIC_API UBSInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBSInventoryComponent();

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FBSInventoryChangedSignature OnInventoryChanged;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool AddItemDefinition(UBSItemDefinition* ItemDefinition, int32 Quantity);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool AddItemStack(const FBSItemStack& Stack);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveItemById(FName ItemId, int32 Quantity);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool ConsumeRequiredItems(const TArray<FBSRequiredItem>& Requirements);

	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool HasRequiredItems(const TArray<FBSRequiredItem>& Requirements) const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetQuantityById(FName ItemId) const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	const TArray<FBSItemStack>& GetItems() const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void ClearInventory();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void TransferAllItemsTo(UBSInventoryComponent* OtherInventory);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetInventoryItems(const TArray<FBSItemStack>& NewItems);

private:
	UPROPERTY(EditAnywhere, Category = "Inventory")
	TArray<FBSItemStack> Items;
};
