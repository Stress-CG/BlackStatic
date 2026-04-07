#pragma once

#include "CoreMinimal.h"
#include "BlackStaticPhase0Types.generated.h"

class UBSItemDefinition;

UENUM(BlueprintType)
enum class EBSFaction : uint8
{
	Settlements UMETA(DisplayName = "Settlements"),
	Infected UMETA(DisplayName = "Infected"),
	Raiders UMETA(DisplayName = "Raiders"),
	Corporation UMETA(DisplayName = "Corporation"),
	Military UMETA(DisplayName = "Military"),
	Neutral UMETA(DisplayName = "Neutral")
};

UENUM(BlueprintType)
enum class EBSItemCategory : uint8
{
	Quest UMETA(DisplayName = "Quest"),
	Food UMETA(DisplayName = "Food"),
	Water UMETA(DisplayName = "Water"),
	Medical UMETA(DisplayName = "Medical"),
	Tool UMETA(DisplayName = "Tool"),
	Weapon UMETA(DisplayName = "Weapon"),
	Ammo UMETA(DisplayName = "Ammo"),
	Barter UMETA(DisplayName = "Barter"),
	Settlement UMETA(DisplayName = "Settlement")
};

UENUM(BlueprintType)
enum class EBSNoiseType : uint8
{
	Movement UMETA(DisplayName = "Movement"),
	Door UMETA(DisplayName = "Door"),
	Combat UMETA(DisplayName = "Combat"),
	Tool UMETA(DisplayName = "Tool"),
	Objective UMETA(DisplayName = "Objective"),
	Ambient UMETA(DisplayName = "Ambient")
};

UENUM(BlueprintType)
enum class EBSObjectiveType : uint8
{
	WaterPowerRestoration UMETA(DisplayName = "Water & Power Restoration"),
	SupplyRun UMETA(DisplayName = "Supply Run"),
	EquipmentRetrieval UMETA(DisplayName = "Equipment Retrieval")
};

UENUM(BlueprintType)
enum class EBSObjectiveState : uint8
{
	None UMETA(DisplayName = "None"),
	Accepted UMETA(DisplayName = "Accepted"),
	InProgress UMETA(DisplayName = "In Progress"),
	SiteCompleted UMETA(DisplayName = "Site Completed"),
	ExtractReady UMETA(DisplayName = "Ready To Extract"),
	Completed UMETA(DisplayName = "Completed"),
	Failed UMETA(DisplayName = "Failed")
};

USTRUCT(BlueprintType)
struct FBSItemStack
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TSoftObjectPtr<UBSItemDefinition> ItemDefinition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FName ItemId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item", meta = (ClampMin = "1"))
	int32 Quantity = 1;

	bool IsValid() const
	{
		return Quantity > 0 && (!ItemId.IsNone() || ItemDefinition.IsValid() || ItemDefinition.ToSoftObjectPath().IsValid());
	}
};

USTRUCT(BlueprintType)
struct FBSRequiredItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	TSoftObjectPtr<UBSItemDefinition> ItemDefinition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	FName ItemId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task", meta = (ClampMin = "1"))
	int32 Quantity = 1;
};

USTRUCT(BlueprintType)
struct FBSTaskReward
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	int32 USD = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	float Reputation = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	float VendorTrust = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	int32 SettlementPower = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	int32 SettlementWater = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	int32 SettlementInfrastructure = 0;
};

USTRUCT(BlueprintType)
struct FBSNoiseEvent
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Noise")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Noise")
	float Loudness = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Noise")
	float RadiusUnits = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Noise")
	EBSNoiseType NoiseType = EBSNoiseType::Ambient;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Noise")
	EBSFaction InstigatorFaction = EBSFaction::Neutral;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Noise")
	FName NoiseTag = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Noise")
	TWeakObjectPtr<AActor> InstigatorActor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Noise")
	float GameTimeSeconds = 0.0f;
};

USTRUCT(BlueprintType)
struct FBSSurvivorVitals
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vitals")
	float Health = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vitals")
	float Hunger = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vitals")
	float Stamina = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vitals")
	float Stress = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vitals")
	bool bAlive = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vitals")
	float BountyPenaltySecondsRemaining = 0.0f;
};

USTRUCT(BlueprintType)
struct FBSSettlementState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement")
	int32 USD = 250;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement")
	float Reputation = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement")
	float VendorTrust = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement")
	int32 UnlockTier = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement")
	int32 Power = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement")
	int32 Water = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement")
	int32 Infrastructure = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement")
	bool bClassifiedRaider = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement")
	TArray<FBSItemStack> PersistentStash;
};
