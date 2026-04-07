#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Phase0/BlackStaticPhase0Statics.h"
#include "Phase0/Components/BSInventoryComponent.h"
#include "Phase0/Data/BSItemDefinition.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBlackStaticPhase0NoiseTest, "BlackStatic.Phase0.MovementNoise", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FBlackStaticPhase0NoiseTest::RunTest(const FString& Parameters)
{
	const float CrouchedNoise = UBSPhase0Statics::ComputeMovementNoiseLoudness(150.0f, 300.0f, true, false);
	const float WalkingNoise = UBSPhase0Statics::ComputeMovementNoiseLoudness(150.0f, 300.0f, false, false);
	const float SprintNoise = UBSPhase0Statics::ComputeMovementNoiseLoudness(300.0f, 300.0f, false, true);

	TestTrue(TEXT("Crouched movement is quieter than a normal walk."), CrouchedNoise < WalkingNoise);
	TestTrue(TEXT("Sprinting is louder than walking."), SprintNoise > WalkingNoise);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBlackStaticPhase0UnlockTierTest, "BlackStatic.Phase0.UnlockTiers", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FBlackStaticPhase0UnlockTierTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Default reputation starts at tier 0."), UBSPhase0Statics::ComputeUnlockTierFromReputation(0.0f), 0);
	TestEqual(TEXT("Early positive reputation unlocks tier 1."), UBSPhase0Statics::ComputeUnlockTierFromReputation(25.0f), 1);
	TestEqual(TEXT("Mid reputation unlocks tier 2."), UBSPhase0Statics::ComputeUnlockTierFromReputation(70.0f), 2);
	TestEqual(TEXT("High reputation unlocks tier 3."), UBSPhase0Statics::ComputeUnlockTierFromReputation(140.0f), 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBlackStaticPhase0InventoryRequirementTest, "BlackStatic.Phase0.InventoryRequirements", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FBlackStaticPhase0InventoryRequirementTest::RunTest(const FString& Parameters)
{
	UBSInventoryComponent* Inventory = NewObject<UBSInventoryComponent>();
	UBSItemDefinition* Battery = NewObject<UBSItemDefinition>();
	UBSItemDefinition* Filter = NewObject<UBSItemDefinition>();

	Battery->ItemId = TEXT("Battery");
	Filter->ItemId = TEXT("Filter");

	TestTrue(TEXT("Battery stack is added."), Inventory->AddItemDefinition(Battery, 1));
	TestTrue(TEXT("Filter stack is added."), Inventory->AddItemDefinition(Filter, 1));

	TArray<FBSRequiredItem> Requirements;
	Requirements.Add({ Battery, Battery->ResolveItemId(), 1 });
	Requirements.Add({ Filter, Filter->ResolveItemId(), 1 });

	TestTrue(TEXT("Inventory satisfies the restoration requirements."), Inventory->HasRequiredItems(Requirements));
	TestTrue(TEXT("Required items are consumed successfully."), Inventory->ConsumeRequiredItems(Requirements));
	TestEqual(TEXT("Battery is consumed."), Inventory->GetQuantityById(Battery->ResolveItemId()), 0);
	TestEqual(TEXT("Filter is consumed."), Inventory->GetQuantityById(Filter->ResolveItemId()), 0);
	return true;
}

#endif
