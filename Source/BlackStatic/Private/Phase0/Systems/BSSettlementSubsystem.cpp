#include "Phase0/Systems/BSSettlementSubsystem.h"

#include "Kismet/GameplayStatics.h"
#include "Phase0/BlackStaticPhase0Statics.h"
#include "Phase0/Components/BSInventoryComponent.h"
#include "Phase0/Data/BSTaskDefinition.h"
#include "Phase0/Systems/BSSaveGame.h"

void UBSSettlementSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadOrCreateSave();
}

void UBSSettlementSubsystem::Deinitialize()
{
	SaveSettlementState();
	Super::Deinitialize();
}

const FBSSettlementState& UBSSettlementSubsystem::GetSettlementState() const
{
	return CurrentState;
}

void UBSSettlementSubsystem::ResetPrototypeProgress()
{
	CurrentState = FBSSettlementState();
	BroadcastAndSave();
}

void UBSSettlementSubsystem::AddUSD(const int32 Amount)
{
	CurrentState.USD = FMath::Max(0, CurrentState.USD + Amount);
	BroadcastAndSave();
}

void UBSSettlementSubsystem::AdjustReputation(const float Delta)
{
	CurrentState.Reputation += Delta;
	CurrentState.UnlockTier = UBSPhase0Statics::ComputeUnlockTierFromReputation(CurrentState.Reputation);
	if (CurrentState.Reputation <= -50.0f)
	{
		CurrentState.bClassifiedRaider = true;
	}

	BroadcastAndSave();
}

void UBSSettlementSubsystem::AdjustVendorTrust(const float Delta)
{
	CurrentState.VendorTrust += Delta;
	BroadcastAndSave();
}

void UBSSettlementSubsystem::AddToStash(const FBSItemStack& Stack)
{
	UBSPhase0Statics::AddOrMergeItemStack(CurrentState.PersistentStash, Stack);
	BroadcastAndSave();
}

void UBSSettlementSubsystem::DepositInventory(UBSInventoryComponent* Inventory)
{
	if (!Inventory)
	{
		return;
	}

	for (const FBSItemStack& Stack : Inventory->GetItems())
	{
		UBSPhase0Statics::AddOrMergeItemStack(CurrentState.PersistentStash, Stack);
	}

	Inventory->ClearInventory();
	BroadcastAndSave();
}

void UBSSettlementSubsystem::ApplyTaskReward(UBSTaskDefinition* TaskDefinition)
{
	if (!TaskDefinition)
	{
		return;
	}

	const FBSTaskReward& Reward = TaskDefinition->Reward;
	CurrentState.USD += Reward.USD;
	CurrentState.Reputation += Reward.Reputation;
	CurrentState.VendorTrust += Reward.VendorTrust;
	CurrentState.Power += Reward.SettlementPower;
	CurrentState.Water += Reward.SettlementWater;
	CurrentState.Infrastructure += Reward.SettlementInfrastructure;
	CurrentState.UnlockTier = UBSPhase0Statics::ComputeUnlockTierFromReputation(CurrentState.Reputation);
	BroadcastAndSave();
}

void UBSSettlementSubsystem::RegisterPlayerKillPenalty()
{
	CurrentState.Reputation -= 35.0f;
	CurrentState.VendorTrust -= 20.0f;
	CurrentState.UnlockTier = UBSPhase0Statics::ComputeUnlockTierFromReputation(CurrentState.Reputation);
	CurrentState.bClassifiedRaider = CurrentState.Reputation <= -50.0f;
	BroadcastAndSave();
}

void UBSSettlementSubsystem::SaveSettlementState()
{
	if (!CachedSaveGame)
	{
		CachedSaveGame = Cast<UBSSaveGame>(UGameplayStatics::CreateSaveGameObject(UBSSaveGame::StaticClass()));
	}

	if (!CachedSaveGame)
	{
		return;
	}

	CachedSaveGame->SettlementState = CurrentState;
	UGameplayStatics::SaveGameToSlot(CachedSaveGame, SaveSlotName, SaveUserIndex);
}

void UBSSettlementSubsystem::BroadcastAndSave()
{
	OnSettlementChanged.Broadcast();
	SaveSettlementState();
}

void UBSSettlementSubsystem::LoadOrCreateSave()
{
	if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, SaveUserIndex))
	{
		CachedSaveGame = Cast<UBSSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, SaveUserIndex));
	}

	if (!CachedSaveGame)
	{
		CachedSaveGame = Cast<UBSSaveGame>(UGameplayStatics::CreateSaveGameObject(UBSSaveGame::StaticClass()));
	}

	CurrentState = CachedSaveGame ? CachedSaveGame->SettlementState : FBSSettlementState();
}
