#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Phase0/BlackStaticPhase0Types.h"
#include "BSSettlementSubsystem.generated.h"

class UBSInventoryComponent;
class UBSSaveGame;
class UBSTaskDefinition;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FBSSettlementChangedSignature);

UCLASS()
class BLACKSTATIC_API UBSSettlementSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UPROPERTY(BlueprintAssignable, Category = "Settlement")
	FBSSettlementChangedSignature OnSettlementChanged;

	UFUNCTION(BlueprintPure, Category = "Settlement")
	const FBSSettlementState& GetSettlementState() const;

	UFUNCTION(BlueprintCallable, Category = "Settlement")
	void ResetPrototypeProgress();

	UFUNCTION(BlueprintCallable, Category = "Settlement")
	void AddUSD(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Settlement")
	void AdjustReputation(float Delta);

	UFUNCTION(BlueprintCallable, Category = "Settlement")
	void AdjustVendorTrust(float Delta);

	UFUNCTION(BlueprintCallable, Category = "Settlement")
	void AddToStash(const FBSItemStack& Stack);

	UFUNCTION(BlueprintCallable, Category = "Settlement")
	void DepositInventory(UBSInventoryComponent* Inventory);

	UFUNCTION(BlueprintCallable, Category = "Settlement")
	bool WithdrawAllStashToInventory(UBSInventoryComponent* Inventory);

	UFUNCTION(BlueprintPure, Category = "Settlement")
	bool HasPersistentStashItems() const;

	UFUNCTION(BlueprintCallable, Category = "Settlement")
	void ApplyTaskReward(UBSTaskDefinition* TaskDefinition);

	UFUNCTION(BlueprintCallable, Category = "Settlement")
	void RegisterPlayerKillPenalty();

	UFUNCTION(BlueprintCallable, Category = "Settlement")
	void SaveSettlementState();

private:
	void BroadcastAndSave();
	void LoadOrCreateSave();

	UPROPERTY()
	FBSSettlementState CurrentState;

	UPROPERTY()
	TObjectPtr<UBSSaveGame> CachedSaveGame;

	FString SaveSlotName = TEXT("BlackStaticPhase0Settlement");
	int32 SaveUserIndex = 0;
};
