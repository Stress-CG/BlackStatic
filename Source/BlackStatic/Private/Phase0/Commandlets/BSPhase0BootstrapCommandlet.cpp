#include "Phase0/Commandlets/BSPhase0BootstrapCommandlet.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "NavMesh/NavMeshBoundsVolume.h"
#include "Phase0/Actors/BSExtractionPoint.h"
#include "Phase0/Actors/BSInfectedCharacter.h"
#include "Phase0/Actors/BSObjectivePickup.h"
#include "Phase0/Actors/BSStashActor.h"
#include "Phase0/Actors/BSTaskBoardActor.h"
#include "Phase0/Actors/BSWaterPowerObjectiveActor.h"
#include "Phase0/Data/BSItemDefinition.h"
#include "Phase0/Data/BSTaskDefinition.h"
#include "Phase0\Framework/BSPhase0GameMode.h"
#include "UObject/SavePackage.h"

#if WITH_EDITOR
#include "FileHelpers.h"
#endif

namespace
{
	static const FString ItemRoot = TEXT("/Game/Phase0/Data/Items");
	static const FString TaskRoot = TEXT("/Game/Phase0/Data/Tasks");
	static const FString MapPath = TEXT("/Game/Phase0/Maps/MAP_Phase0_Prototype");
	static const FString SpawnedActorPrefix = TEXT("BSP0_");

	template <typename TAssetType>
	TAssetType* CreateOrLoadAsset(const FString& PackageName, const FString& AssetName)
	{
		UPackage* Package = CreatePackage(*PackageName);
		Package->FullyLoad();

		TAssetType* Asset = FindObject<TAssetType>(Package, *AssetName);
		if (!Asset)
		{
			Asset = NewObject<TAssetType>(Package, *AssetName, RF_Public | RF_Standalone);
			FAssetRegistryModule::AssetCreated(Asset);
		}

		return Asset;
	}

	bool SaveAsset(UObject* Asset)
	{
		if (!Asset)
		{
			return false;
		}

		UPackage* Package = Asset->GetPackage();
		Package->MarkPackageDirty();

		const FString FilePath = FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetAssetPackageExtension());
		FSavePackageArgs SaveArgs;
		SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
		SaveArgs.SaveFlags = SAVE_NoError;
		return UPackage::SavePackage(Package, Asset, *FilePath, SaveArgs);
	}

#if WITH_EDITOR
	void DestroyExistingBootstrapActors(UWorld* World)
	{
		if (!World)
		{
			return;
		}

		TArray<AActor*> ActorsToDestroy;
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (!Actor)
			{
				continue;
			}

			if (Actor->GetActorLabel().StartsWith(SpawnedActorPrefix))
			{
				ActorsToDestroy.Add(Actor);
			}
		}

		for (AActor* Actor : ActorsToDestroy)
		{
			World->EditorDestroyActor(Actor, true);
		}
	}

	AStaticMeshActor* SpawnMeshActor(UWorld* World, UStaticMesh* StaticMesh, const FString& Label, const FVector& Location, const FVector& Scale, const FRotator& Rotation = FRotator::ZeroRotator)
	{
		AStaticMeshActor* Actor = World->SpawnActor<AStaticMeshActor>(Location, Rotation);
		if (!Actor)
		{
			return nullptr;
		}

		Actor->SetActorLabel(Label);
		Actor->GetStaticMeshComponent()->SetStaticMesh(StaticMesh);
		Actor->SetActorScale3D(Scale);
		return Actor;
	}
#endif
}

UBSPhase0BootstrapCommandlet::UBSPhase0BootstrapCommandlet()
{
	IsClient = false;
	IsServer = false;
	IsEditor = true;
	LogToConsole = true;
	ShowErrorCount = true;
}

int32 UBSPhase0BootstrapCommandlet::Main(const FString& Params)
{
#if !WITH_EDITOR
	return 1;
#else
	const bool bCreateAssets = !FParse::Param(*Params, TEXT("MapOnly"));
	const bool bCreateMap = !FParse::Param(*Params, TEXT("AssetsOnly"));

	IFileManager::Get().MakeDirectory(*(FPaths::ProjectContentDir() / TEXT("Phase0/Data/Items")), true);
	IFileManager::Get().MakeDirectory(*(FPaths::ProjectContentDir() / TEXT("Phase0/Data/Tasks")), true);
	IFileManager::Get().MakeDirectory(*(FPaths::ProjectContentDir() / TEXT("Phase0/Maps")), true);

	UBSItemDefinition* BatteryDefinition = nullptr;
	UBSItemDefinition* FilterDefinition = nullptr;
	UBSItemDefinition* WiresDefinition = nullptr;
	UBSItemDefinition* BandageDefinition = nullptr;
	UBSItemDefinition* WaterDefinition = nullptr;

	if (bCreateAssets)
	{
		BatteryDefinition = CreateOrLoadAsset<UBSItemDefinition>(ItemRoot / TEXT("DA_BS_Battery"), TEXT("DA_BS_Battery"));
		BatteryDefinition->ItemId = TEXT("Battery");
		BatteryDefinition->DisplayName = FText::FromString(TEXT("Battery"));
		BatteryDefinition->Description = FText::FromString(TEXT("A restored power cell needed for water and power repairs."));
		BatteryDefinition->Category = EBSItemCategory::Quest;
		BatteryDefinition->BarterValue = 35;
		BatteryDefinition->SettlementValue = 20;
		BatteryDefinition->bQuestCritical = true;
		SaveAsset(BatteryDefinition);

		FilterDefinition = CreateOrLoadAsset<UBSItemDefinition>(ItemRoot / TEXT("DA_BS_Filter"), TEXT("DA_BS_Filter"));
		FilterDefinition->ItemId = TEXT("Filter");
		FilterDefinition->DisplayName = FText::FromString(TEXT("Filter"));
		FilterDefinition->Description = FText::FromString(TEXT("A water filtration cartridge required for settlement pump repairs."));
		FilterDefinition->Category = EBSItemCategory::Quest;
		FilterDefinition->BarterValue = 25;
		FilterDefinition->SettlementValue = 20;
		FilterDefinition->bQuestCritical = true;
		SaveAsset(FilterDefinition);

		WiresDefinition = CreateOrLoadAsset<UBSItemDefinition>(ItemRoot / TEXT("DA_BS_Wires"), TEXT("DA_BS_Wires"));
		WiresDefinition->ItemId = TEXT("Wires");
		WiresDefinition->DisplayName = FText::FromString(TEXT("Wires"));
		WiresDefinition->Description = FText::FromString(TEXT("Multi-use electrical wire that can be traded or used for infrastructure upgrades."));
		WiresDefinition->Category = EBSItemCategory::Settlement;
		WiresDefinition->USDValue = 15;
		WiresDefinition->BarterValue = 12;
		WiresDefinition->SettlementValue = 10;
		SaveAsset(WiresDefinition);

		BandageDefinition = CreateOrLoadAsset<UBSItemDefinition>(ItemRoot / TEXT("DA_BS_Bandage"), TEXT("DA_BS_Bandage"));
		BandageDefinition->ItemId = TEXT("Bandage");
		BandageDefinition->DisplayName = FText::FromString(TEXT("Bandage"));
		BandageDefinition->Description = FText::FromString(TEXT("Basic wound treatment. Commonly sold for USD."));
		BandageDefinition->Category = EBSItemCategory::Medical;
		BandageDefinition->USDValue = 18;
		BandageDefinition->bConsumable = true;
		SaveAsset(BandageDefinition);

		WaterDefinition = CreateOrLoadAsset<UBSItemDefinition>(ItemRoot / TEXT("DA_BS_WaterBottle"), TEXT("DA_BS_WaterBottle"));
		WaterDefinition->ItemId = TEXT("WaterBottle");
		WaterDefinition->DisplayName = FText::FromString(TEXT("Water Bottle"));
		WaterDefinition->Description = FText::FromString(TEXT("Basic hydration supply bought with USD inside settlements."));
		WaterDefinition->Category = EBSItemCategory::Water;
		WaterDefinition->USDValue = 10;
		WaterDefinition->bConsumable = true;
		SaveAsset(WaterDefinition);

		UBSTaskDefinition* WaterPowerTask = CreateOrLoadAsset<UBSTaskDefinition>(TaskRoot / TEXT("DA_BS_WaterPowerRestoration"), TEXT("DA_BS_WaterPowerRestoration"));
		WaterPowerTask->TaskId = TEXT("WaterPowerRestoration");
		WaterPowerTask->DisplayName = FText::FromString(TEXT("Water & Power Restoration"));
		WaterPowerTask->Description = FText::FromString(TEXT("Leave the settlement, reach the maintenance site, recover a battery and filter, restore utilities, and extract alive."));
		WaterPowerTask->ObjectiveType = EBSObjectiveType::WaterPowerRestoration;
		WaterPowerTask->RequiredItems =
		{
			{ BatteryDefinition, BatteryDefinition->ResolveItemId(), 1 },
			{ FilterDefinition, FilterDefinition->ResolveItemId(), 1 }
		};
		WaterPowerTask->Reward.USD = 120;
		WaterPowerTask->Reward.Reputation = 25.0f;
		WaterPowerTask->Reward.VendorTrust = 12.0f;
		WaterPowerTask->Reward.SettlementPower = 1;
		WaterPowerTask->Reward.SettlementWater = 1;
		WaterPowerTask->Reward.SettlementInfrastructure = 1;
		WaterPowerTask->BaseThreatCount = 4;
		WaterPowerTask->bRepeatable = true;
		WaterPowerTask->bRequireExtractionAfterSiteCompletion = true;
		SaveAsset(WaterPowerTask);
	}

	if (!bCreateMap)
	{
		return 0;
	}

	if (!BatteryDefinition)
	{
		BatteryDefinition = LoadObject<UBSItemDefinition>(nullptr, TEXT("/Game/Phase0/Data/Items/DA_BS_Battery.DA_BS_Battery"));
	}

	if (!FilterDefinition)
	{
		FilterDefinition = LoadObject<UBSItemDefinition>(nullptr, TEXT("/Game/Phase0/Data/Items/DA_BS_Filter.DA_BS_Filter"));
	}

	UBSTaskDefinition* WaterPowerTask = LoadObject<UBSTaskDefinition>(nullptr, TEXT("/Game/Phase0/Data/Tasks/DA_BS_WaterPowerRestoration.DA_BS_WaterPowerRestoration"));
	if (!BatteryDefinition || !FilterDefinition || !WaterPowerTask)
	{
		UE_LOG(LogTemp, Error, TEXT("Phase 0 assets are missing. Run the bootstrap with asset creation enabled."));
		return 1;
	}

	const FString MapFilename = FPackageName::LongPackageNameToFilename(MapPath, FPackageName::GetMapPackageExtension());
	UWorld* PrototypeWorld = nullptr;
	if (FPackageName::DoesPackageExist(MapPath))
	{
		PrototypeWorld = UEditorLoadingAndSavingUtils::LoadMap(MapFilename);
	}
	else
	{
		PrototypeWorld = UEditorLoadingAndSavingUtils::NewBlankMap(false);
	}

	if (!PrototypeWorld)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create or load the Phase 0 prototype map."));
		return 1;
	}

	DestroyExistingBootstrapActors(PrototypeWorld);
	PrototypeWorld->GetWorldSettings()->DefaultGameMode = ABSPhase0GameMode::StaticClass();
	PrototypeWorld->GetWorldSettings()->Modify();

	UStaticMesh* PlaneMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));
	UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (!PlaneMesh || !CubeMesh)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load required engine meshes for the prototype map."));
		return 1;
	}

	APlayerStart* PlayerStart = PrototypeWorld->SpawnActor<APlayerStart>(FVector(0.0f, 0.0f, 120.0f), FRotator::ZeroRotator);
	PlayerStart->SetActorLabel(SpawnedActorPrefix + TEXT("PlayerStart"));

	ANavMeshBoundsVolume* NavBounds = PrototypeWorld->SpawnActor<ANavMeshBoundsVolume>(FVector(2200.0f, 0.0f, 200.0f), FRotator::ZeroRotator);
	NavBounds->SetActorScale3D(FVector(20.0f, 12.0f, 4.0f));
	NavBounds->SetActorLabel(SpawnedActorPrefix + TEXT("NavMeshBounds"));

	AStaticMeshActor* Ground = SpawnMeshActor(PrototypeWorld, PlaneMesh, SpawnedActorPrefix + TEXT("Ground"), FVector(2200.0f, 0.0f, 0.0f), FVector(45.0f, 18.0f, 1.0f));
	if (Ground)
	{
		Ground->GetStaticMeshComponent()->SetMobility(EComponentMobility::Static);
	}

	AStaticMeshActor* SettlementWallA = SpawnMeshActor(PrototypeWorld, CubeMesh, SpawnedActorPrefix + TEXT("SettlementWallA"), FVector(900.0f, 350.0f, 100.0f), FVector(2.0f, 0.15f, 2.0f));
	AStaticMeshActor* SettlementWallB = SpawnMeshActor(PrototypeWorld, CubeMesh, SpawnedActorPrefix + TEXT("SettlementWallB"), FVector(900.0f, -350.0f, 100.0f), FVector(2.0f, 0.15f, 2.0f));
	AStaticMeshActor* RoadBlockA = SpawnMeshActor(PrototypeWorld, CubeMesh, SpawnedActorPrefix + TEXT("RoadBlockA"), FVector(2350.0f, 260.0f, 90.0f), FVector(1.6f, 0.5f, 1.8f));
	AStaticMeshActor* RoadBlockB = SpawnMeshActor(PrototypeWorld, CubeMesh, SpawnedActorPrefix + TEXT("RoadBlockB"), FVector(2600.0f, -260.0f, 90.0f), FVector(1.6f, 0.5f, 1.8f));
	AStaticMeshActor* SiteBarrierA = SpawnMeshActor(PrototypeWorld, CubeMesh, SpawnedActorPrefix + TEXT("SiteBarrierA"), FVector(3950.0f, 420.0f, 110.0f), FVector(2.4f, 0.25f, 2.2f));
	AStaticMeshActor* SiteBarrierB = SpawnMeshActor(PrototypeWorld, CubeMesh, SpawnedActorPrefix + TEXT("SiteBarrierB"), FVector(3950.0f, -420.0f, 110.0f), FVector(2.4f, 0.25f, 2.2f));
	if (SettlementWallA) { SettlementWallA->GetStaticMeshComponent()->SetMobility(EComponentMobility::Static); }
	if (SettlementWallB) { SettlementWallB->GetStaticMeshComponent()->SetMobility(EComponentMobility::Static); }
	if (RoadBlockA) { RoadBlockA->GetStaticMeshComponent()->SetMobility(EComponentMobility::Static); }
	if (RoadBlockB) { RoadBlockB->GetStaticMeshComponent()->SetMobility(EComponentMobility::Static); }
	if (SiteBarrierA) { SiteBarrierA->GetStaticMeshComponent()->SetMobility(EComponentMobility::Static); }
	if (SiteBarrierB) { SiteBarrierB->GetStaticMeshComponent()->SetMobility(EComponentMobility::Static); }

	ADirectionalLight* DirectionalLight = PrototypeWorld->SpawnActor<ADirectionalLight>(FVector(0.0f, 0.0f, 600.0f), FRotator(-50.0f, 35.0f, 0.0f));
	DirectionalLight->SetActorLabel(SpawnedActorPrefix + TEXT("DirectionalLight"));

	ASkyLight* SkyLight = PrototypeWorld->SpawnActor<ASkyLight>(FVector(0.0f, 0.0f, 250.0f), FRotator::ZeroRotator);
	SkyLight->SetActorLabel(SpawnedActorPrefix + TEXT("SkyLight"));

	ABSTaskBoardActor* TaskBoard = PrototypeWorld->SpawnActor<ABSTaskBoardActor>(FVector(180.0f, 0.0f, 40.0f), FRotator::ZeroRotator);
	TaskBoard->SetActorLabel(SpawnedActorPrefix + TEXT("TaskBoard"));
	TaskBoard->TaskDefinition = WaterPowerTask;

	ABSStashActor* Stash = PrototypeWorld->SpawnActor<ABSStashActor>(FVector(360.0f, 110.0f, 40.0f), FRotator::ZeroRotator);
	Stash->SetActorLabel(SpawnedActorPrefix + TEXT("Stash"));

	ABSExtractionPoint* Extraction = PrototypeWorld->SpawnActor<ABSExtractionPoint>(FVector(540.0f, -110.0f, 10.0f), FRotator::ZeroRotator);
	Extraction->SetActorLabel(SpawnedActorPrefix + TEXT("Extraction"));

	ABSWaterPowerObjectiveActor* ObjectiveActor = PrototypeWorld->SpawnActor<ABSWaterPowerObjectiveActor>(FVector(4100.0f, 0.0f, 30.0f), FRotator::ZeroRotator);
	ObjectiveActor->SetActorLabel(SpawnedActorPrefix + TEXT("WaterPowerObjective"));
	ObjectiveActor->TaskDefinition = WaterPowerTask;
	ObjectiveActor->InfectedClass = ABSInfectedCharacter::StaticClass();
	ObjectiveActor->ObjectiveDefenseRadius = 1200.0f;

	TaskBoard->LinkedObjective = ObjectiveActor;

	ABSObjectivePickup* BatteryPickup = PrototypeWorld->SpawnActor<ABSObjectivePickup>(FVector(3800.0f, -220.0f, 35.0f), FRotator::ZeroRotator);
	BatteryPickup->SetActorLabel(SpawnedActorPrefix + TEXT("BatteryPickup"));
	BatteryPickup->ItemDefinition = BatteryDefinition;

	ABSObjectivePickup* FilterPickup = PrototypeWorld->SpawnActor<ABSObjectivePickup>(FVector(4375.0f, 230.0f, 35.0f), FRotator::ZeroRotator);
	FilterPickup->SetActorLabel(SpawnedActorPrefix + TEXT("FilterPickup"));
	FilterPickup->ItemDefinition = FilterDefinition;

	if (!UEditorLoadingAndSavingUtils::SaveMap(PrototypeWorld, MapPath))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to save the Phase 0 prototype map."));
		return 1;
	}

	UE_LOG(LogTemp, Display, TEXT("Phase 0 prototype content is ready at %s"), *MapPath);
	return 0;
#endif
}
